#include <windows.h>
#include <stdio.h>
#include <iostream>

BYTE JmpOriginal[5] = { 0xE9, 0, 0, 0, 0 };  // 用于跳转到MyMessageBoxA的指令，0xE9代表JMP指令
BYTE OldCode[5] = { 0 };                      // 存储原始MessageBoxA的前5个字节
FARPROC MessageBoxAddress;                    // MessageBoxA的函数地址
void* Trampoline;                             // 桥接函数地址

// 自定义的MessageBoxA函数
int WINAPI MyMessageBoxA(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
    printf("MessageBoxA 已经被Hook\n");  // 打印被Hook的信息

    // 使用桥接函数调用原始的MessageBoxA，这里需要类型转换
    int ret = ((int (WINAPI*)(HWND, LPCTSTR, LPCTSTR, UINT))Trampoline)(hWnd, lpText, lpCaption, uType);
    return ret;
}

void InlineHook()
{
    HMODULE hModule_User32 = LoadLibraryA("user32.dll");  // 加载user32.dll模块
    MessageBoxAddress = GetProcAddress(hModule_User32, "MessageBoxA");  // 获取MessageBoxA的函数地址

    DWORD JmpAddress = (DWORD)MyMessageBoxA - (DWORD)MessageBoxAddress - 5;  // 计算跳转到MyMessageBoxA的地址
    memcpy(&JmpOriginal[1], &JmpAddress, 4);  // 将跳转地址复制到JmpOriginal的第二个字节

    ReadProcessMemory(GetCurrentProcess(), MessageBoxAddress, OldCode, 5, NULL);  // 读取并保存MessageBoxA的前5个字节

    // 分配10个字节的内存空间作为桥接函数
    Trampoline = VirtualAlloc(NULL, 10, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memcpy(Trampoline, OldCode, 5);  // 复制MessageBoxA的前5个字节到桥接函数

    // 计算并写入桥接函数的跳回地址
    DWORD jmpBackAddr = (DWORD)MessageBoxAddress + 5 - (DWORD)Trampoline - 5;
    memcpy((void*)((DWORD)Trampoline + 5), &JmpOriginal[0], 5);
    memcpy((void*)((DWORD)Trampoline + 6), &jmpBackAddr, 4);

    DWORD dwOldProtect;
    // 修改MessageBoxA的前5个字节的页属性，使其可读可写可执行
    VirtualProtect(MessageBoxAddress, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

    // 替换MessageBoxA的前5个字节为跳转到MyMessageBoxA的指令
    WriteProcessMemory(GetCurrentProcess(), MessageBoxAddress, &JmpOriginal[0], 5, NULL);

    // 恢复MessageBoxA的前5个字节的原始页属性
    VirtualProtect(MessageBoxAddress, 5, dwOldProtect, &dwOldProtect);
}

void main()
{
    InlineHook();  // 实施Inline Hook
    MessageBoxA(NULL, "Hello World", "Title", MB_OK);  // 调用MessageBoxA函数
}
