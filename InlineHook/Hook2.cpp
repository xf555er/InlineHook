#include <windows.h>
#include <stdio.h>
#include <iostream>

BYTE JmpOriginal[5] = { 0xE9, 0, 0, 0, 0 };  // ������ת��MyMessageBoxA��ָ�0xE9����JMPָ��
BYTE OldCode[5] = { 0 };                      // �洢ԭʼMessageBoxA��ǰ5���ֽ�
FARPROC MessageBoxAddress;                    // MessageBoxA�ĺ�����ַ
void* Trampoline;                             // �ŽӺ�����ַ

// �Զ����MessageBoxA����
int WINAPI MyMessageBoxA(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
    printf("MessageBoxA �Ѿ���Hook\n");  // ��ӡ��Hook����Ϣ

    // ʹ���ŽӺ�������ԭʼ��MessageBoxA��������Ҫ����ת��
    int ret = ((int (WINAPI*)(HWND, LPCTSTR, LPCTSTR, UINT))Trampoline)(hWnd, lpText, lpCaption, uType);
    return ret;
}

void InlineHook()
{
    HMODULE hModule_User32 = LoadLibraryA("user32.dll");  // ����user32.dllģ��
    MessageBoxAddress = GetProcAddress(hModule_User32, "MessageBoxA");  // ��ȡMessageBoxA�ĺ�����ַ

    DWORD JmpAddress = (DWORD)MyMessageBoxA - (DWORD)MessageBoxAddress - 5;  // ������ת��MyMessageBoxA�ĵ�ַ
    memcpy(&JmpOriginal[1], &JmpAddress, 4);  // ����ת��ַ���Ƶ�JmpOriginal�ĵڶ����ֽ�

    ReadProcessMemory(GetCurrentProcess(), MessageBoxAddress, OldCode, 5, NULL);  // ��ȡ������MessageBoxA��ǰ5���ֽ�

    // ����10���ֽڵ��ڴ�ռ���Ϊ�ŽӺ���
    Trampoline = VirtualAlloc(NULL, 10, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memcpy(Trampoline, OldCode, 5);  // ����MessageBoxA��ǰ5���ֽڵ��ŽӺ���

    // ���㲢д���ŽӺ��������ص�ַ
    DWORD jmpBackAddr = (DWORD)MessageBoxAddress + 5 - (DWORD)Trampoline - 5;
    memcpy((void*)((DWORD)Trampoline + 5), &JmpOriginal[0], 5);
    memcpy((void*)((DWORD)Trampoline + 6), &jmpBackAddr, 4);

    DWORD dwOldProtect;
    // �޸�MessageBoxA��ǰ5���ֽڵ�ҳ���ԣ�ʹ��ɶ���д��ִ��
    VirtualProtect(MessageBoxAddress, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

    // �滻MessageBoxA��ǰ5���ֽ�Ϊ��ת��MyMessageBoxA��ָ��
    WriteProcessMemory(GetCurrentProcess(), MessageBoxAddress, &JmpOriginal[0], 5, NULL);

    // �ָ�MessageBoxA��ǰ5���ֽڵ�ԭʼҳ����
    VirtualProtect(MessageBoxAddress, 5, dwOldProtect, &dwOldProtect);
}

void main()
{
    InlineHook();  // ʵʩInline Hook
    MessageBoxA(NULL, "Hello World", "Title", MB_OK);  // ����MessageBoxA����
}
