#include<Windows.h>
#include<stdio.h>
#include <detours/detours.h>

// ����һ������ָ��OldMessageBoxA��ָ��MessageBoxA����
static int (WINAPI* OldMesssageBoxA)
(
    HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType
    ) = MessageBoxA;


// �Զ��庯��MyFunction0����MessageBoxA������ʱ������ת���������
int WINAPI MyFunction0(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    // ����������У�����ԭ����MessageBoxA��������ʾ�ض�����Ϣ
    return OldMesssageBoxA(NULL, "Hook Success!", "Warming", MB_OKCANCEL);
}

int main() {
    // ��ʼһ���µ�Detour����
    DetourTransactionBegin();
    
    // ����Detour����߳̽���Ӱ��
    DetourUpdateThread(GetCurrentThread());
    
    // ��OldMessageBoxA����ָ���滻ΪMyFunction0��Ҳ����˵��MessageBoxA������ʱ����ת��MyFunction0
    DetourAttach(&(PVOID&)OldMesssageBoxA, MyFunction0);
    
    // �����Ҫ�Ƴ�Hook�����Ե���DetourDetach
    // DetourDetach(&(PVOID&)OldMesssageBoxA, MyFunction0);
    // �ύDetour����
    DetourTransactionCommit();

    // ����MessageBoxA����ʵ���ϻ���ת��MyFunction0
    MessageBoxA(0, 0, 0, 0);

    return 0;
}
