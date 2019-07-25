// HookAnyDemo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include "Hook.h"
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
int main()
{
	SetDstFunc(newFunc);
	HookFunc(MessageBoxA);
	MessageBoxA(NULL, "haha", "okok", MB_OK);
	return 0;
}

