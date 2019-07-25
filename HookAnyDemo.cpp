// HookAnyDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include "Hook.h"
//方式1
void _declspec(naked) newFunc()
{
	/*
				MOV EAX, [ESP+  4 ]  param0
				MOV EAX, [ESP + 8 ]  param1
				MOV EAX, [ESP + 12]  param2

	*/
	__asm
	{
		MOV EAX, [ESP + 4 ]
		MOV EAX, [ESP + 8 ]
		MOV EAX, [ESP + 12]
		MOV EAX, [ESP + 16]
		ret
	}
}

//方式2
void WINAPI  newFunc2()
{
	int param1, param2, param3,param4;

	__asm
	{
		mov eax,[ebp+8]
		mov param1,eax

		mov eax, [ebp + 8+4]
		mov param2, eax

		mov eax, [ebp + 8+4+4]
		mov param3, eax

		mov eax, [ebp + 8 + 4 + 4 +4]
		mov param4, eax
	}

	//干别的事
}
int main()
{
	//2个都可以
	//SetDstFunc(newFunc1);
	SetDstFunc(newFunc2);

	//要挂钩的函数
	HookFunc(MessageBoxA);
	HookFunc(LoadLibraryA);
	HookFunc(GetModuleHandleA);
	HookFunc(MessageBoxW);

	//测试调用
	MessageBoxA(NULL, "haha", "okok", MB_OK);
	GetModuleHandleA("hwll.dll");
	return 0;
}


