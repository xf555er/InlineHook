#include <Windows.h>
#include <stdio.h>

int main() {
	LoadLibraryA("InlineHook_dll.dll");
	getchar();
	MessageBoxA(NULL, "HelloWorld", "���ڱ���", NULL);
	return 0;
}