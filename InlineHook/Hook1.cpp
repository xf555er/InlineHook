#include <windows.h> 
#include <stdio.h>    
#include <iostream>   
#include <tchar.h>    
BYTE NewCode[7] = { 0xB8, 0x0, 0x0, 0x0, 0x0, 0xFF ,0xE0 };  // 新代码，用于Hook
BYTE OldCode[7] = { 0 };                                     // 旧代码，用于保存被Hook函数的原始字节
FARPROC MessageBoxAddress;                                   // MessageBox函数的地址


// 自定义的MessageBoxA函数
int WINAPI MyMessageBoxA(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	printf("MessageBoxA 已经被Hook\n");  // 打印信息
	
	// 在调用原始函数之前，恢复原始代码
	WriteProcessMemory(INVALID_HANDLE_VALUE, (void*)MessageBoxAddress, (void*)OldCode, 7, NULL);

	// 调用原始的MessageBoxA函数
	int ret = MessageBoxA(NULL, "Hello World", "Title", MB_OK);

	// 在调用原始函数之后，再次将Hook代码写入
	WriteProcessMemory(INVALID_HANDLE_VALUE, (void*)MessageBoxAddress, (void*)NewCode, 7, NULL);
	return ret;
}


void InlineHook()
{
	HMODULE hModule_User32 = LoadLibrary("user32.dll");  //加载user32.dll模块
	MessageBoxAddress = GetProcAddress(hModule_User32, "MessageBoxA");  //获取MessageBoxA函数的地址
	printf("MessageBoxA Addr is %x\n", MessageBoxAddress);
	printf("MyMessageBoxA Addr is %x\n", MyMessageBoxA);

	// 读取MessageBoxA函数的前7个字节，并保存在OldCode数组中
	if (ReadProcessMemory(INVALID_HANDLE_VALUE, MessageBoxAddress, OldCode, 7, NULL) == 0)
	{
		printf("ReadProcessMemory error\n");
		return;
	}

	printf("OldCode is %x%x%x%x%x%x%x\n", OldCode[0], OldCode[1], OldCode[2], OldCode[3], OldCode[4], OldCode[5], OldCode[6]);

	DWORD JmpAddress = (DWORD)MyMessageBoxA - (DWORD)MessageBoxAddress - 5;  // 获取自定义的MessageBoxA函数的地址

	memcpy(&NewCode[1], &JmpAddress, 4);  // 将地址写入到NewCode的第二个字节开始的位置

	DWORD dwOldProtect = 0;  // 用于保存原始页保护
	printf("NewBytes is %x%x%x%x%x\n", NewCode[0], NewCode[1], NewCode[2], NewCode[3], NewCode[4], NewCode[5], NewCode[6]);

	// 使用VirtualProtect函数改变MessageBoxA函数所在页的保护属性，使其可读可写可执行。
	VirtualProtect(MessageBoxAddress, 7, PAGE_EXECUTE_READWRITE,&dwOldProtect);
	
	// 使用WriteProcessMemory函数将我们的Hook代码写入到MessageBoxA函数的开头。
	WriteProcessMemory(INVALID_HANDLE_VALUE, MessageBoxAddress, NewCode, 7,NULL);
	
	// 使用VirtualProtect函数恢复MessageBoxA函数所在页的保护属性。
	VirtualProtect(MessageBoxAddress, 7, dwOldProtect, &dwOldProtect);
}


void main()
{	
	InlineHook();
	MessageBoxA(NULL, "Hello World", "Title", MB_OK);
}