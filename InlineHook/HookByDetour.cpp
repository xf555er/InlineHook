#include<Windows.h>
#include<stdio.h>
#include <detours/detours.h>

// 声明一个函数指针OldMessageBoxA，指向MessageBoxA函数
static int (WINAPI* OldMesssageBoxA)
(
    HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType
    ) = MessageBoxA;


// 自定义函数MyFunction0，当MessageBoxA被调用时，会跳转到这个函数
int WINAPI MyFunction0(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    // 在这个函数中，调用原来的MessageBoxA函数，显示特定的信息
    return OldMesssageBoxA(NULL, "Hook Success!", "Warming", MB_OKCANCEL);
}

int main() {
    // 开始一次新的Detour操作
    DetourTransactionBegin();
    
    // 告诉Detour这个线程将被影响
    DetourUpdateThread(GetCurrentThread());
    
    // 将OldMessageBoxA函数指针替换为MyFunction0，也就是说当MessageBoxA被调用时，跳转到MyFunction0
    DetourAttach(&(PVOID&)OldMesssageBoxA, MyFunction0);
    
    // 如果需要移除Hook，可以调用DetourDetach
    // DetourDetach(&(PVOID&)OldMesssageBoxA, MyFunction0);
    // 提交Detour操作
    DetourTransactionCommit();

    // 调用MessageBoxA，但实际上会跳转到MyFunction0
    MessageBoxA(0, 0, 0, 0);

    return 0;
}
