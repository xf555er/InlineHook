#include <windows.h> 
#include <stdio.h>    
#include <iostream>   
#include <tchar.h>    
BYTE NewCode[7] = { 0xB8, 0x0, 0x0, 0x0, 0x0, 0xFF ,0xE0 };  // �´��룬����Hook
BYTE OldCode[7] = { 0 };                                     // �ɴ��룬���ڱ��汻Hook������ԭʼ�ֽ�
FARPROC MessageBoxAddress;                                   // MessageBox�����ĵ�ַ


// �Զ����MessageBoxA����
int WINAPI MyMessageBoxA(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	printf("MessageBoxA �Ѿ���Hook\n");  // ��ӡ��Ϣ
	
	// �ڵ���ԭʼ����֮ǰ���ָ�ԭʼ����
	WriteProcessMemory(INVALID_HANDLE_VALUE, (void*)MessageBoxAddress, (void*)OldCode, 7, NULL);

	// ����ԭʼ��MessageBoxA����
	int ret = MessageBoxA(NULL, "Hello World", "Title", MB_OK);

	// �ڵ���ԭʼ����֮���ٴν�Hook����д��
	WriteProcessMemory(INVALID_HANDLE_VALUE, (void*)MessageBoxAddress, (void*)NewCode, 7, NULL);
	return ret;
}


void InlineHook()
{
	HMODULE hModule_User32 = LoadLibrary("user32.dll");  //����user32.dllģ��
	MessageBoxAddress = GetProcAddress(hModule_User32, "MessageBoxA");  //��ȡMessageBoxA�����ĵ�ַ
	printf("MessageBoxA Addr is %x\n", MessageBoxAddress);
	printf("MyMessageBoxA Addr is %x\n", MyMessageBoxA);

	// ��ȡMessageBoxA������ǰ7���ֽڣ���������OldCode������
	if (ReadProcessMemory(INVALID_HANDLE_VALUE, MessageBoxAddress, OldCode, 7, NULL) == 0)
	{
		printf("ReadProcessMemory error\n");
		return;
	}

	printf("OldCode is %x%x%x%x%x%x%x\n", OldCode[0], OldCode[1], OldCode[2], OldCode[3], OldCode[4], OldCode[5], OldCode[6]);

	DWORD JmpAddress = (DWORD)MyMessageBoxA - (DWORD)MessageBoxAddress - 5;  // ��ȡ�Զ����MessageBoxA�����ĵ�ַ

	memcpy(&NewCode[1], &JmpAddress, 4);  // ����ַд�뵽NewCode�ĵڶ����ֽڿ�ʼ��λ��

	DWORD dwOldProtect = 0;  // ���ڱ���ԭʼҳ����
	printf("NewBytes is %x%x%x%x%x\n", NewCode[0], NewCode[1], NewCode[2], NewCode[3], NewCode[4], NewCode[5], NewCode[6]);

	// ʹ��VirtualProtect�����ı�MessageBoxA��������ҳ�ı������ԣ�ʹ��ɶ���д��ִ�С�
	VirtualProtect(MessageBoxAddress, 7, PAGE_EXECUTE_READWRITE,&dwOldProtect);
	
	// ʹ��WriteProcessMemory���������ǵ�Hook����д�뵽MessageBoxA�����Ŀ�ͷ��
	WriteProcessMemory(INVALID_HANDLE_VALUE, MessageBoxAddress, NewCode, 7,NULL);
	
	// ʹ��VirtualProtect�����ָ�MessageBoxA��������ҳ�ı������ԡ�
	VirtualProtect(MessageBoxAddress, 7, dwOldProtect, &dwOldProtect);
}


void main()
{	
	InlineHook();
	MessageBoxA(NULL, "Hello World", "Title", MB_OK);
}