#include <Windows.h>
//SrcOpcode
PVOID g_funcAddress[10000] = { 0 };
PBYTE g_funcOpcodes[10000] = { 0 };
DWORD g_funcNums = 0;

//DstFunc
LPVOID g_dstFunc = nullptr;

//DstOpcode
PBYTE g_funcDstOpcdes[10000] = { 0 };

//ShellCode
BYTE JmpshellCode[] = { 232 ,0 ,0, 0 ,0, 233, 46 ,58 ,200 ,17 };
//+7

//Src opCode
PVOID WINAPI FindFunc(LPVOID funcAddress)
{
	for (UINT i = 0; i < g_funcNums; i++)
	{
		if (DWORD(funcAddress) == DWORD(g_funcAddress[i]))
		{
			return g_funcOpcodes[i];
		}
	}
	return nullptr;
}
//Dst opCode
PVOID WINAPI FindFunc2(LPVOID funcAddress)
{
	for (UINT i = 0; i < g_funcNums; i++)
	{
		if (DWORD(funcAddress) == DWORD(g_funcAddress[i]))
		{
			return g_funcDstOpcdes[i];
		}
	}
	return nullptr;
}


PVOID WINAPI AddFunc(LPVOID funcAddress)
{
	PBYTE newMemcode = new BYTE[sizeof(JmpshellCode)];
	PBYTE newMemcodeDstOpcode = new BYTE[sizeof(JmpshellCode)];

	g_funcOpcodes[g_funcNums] = newMemcode;
	g_funcDstOpcdes[g_funcNums] = newMemcodeDstOpcode;

	g_funcAddress[g_funcNums++] = funcAddress;
	return LPVOID(newMemcode);
}


void _declspec(naked) getEip()
{
	__asm
	{
		MOV EAX, [ESP]
		ret
	}
}


struct RETANDSRC
{
	LPVOID SRC, RET;
};

BYTE* g_ret_src[100000] = { 0 };
void WINAPI setRet_Src(LPVOID sc, LPVOID rt)
{
	DWORD thId = GetCurrentThreadId();
	RETANDSRC* tmp = new RETANDSRC;
	tmp->SRC = sc;
	tmp->RET = rt;
	g_ret_src[thId] = (PBYTE)tmp;

}
LPVOID WINAPI getRet()
{
	DWORD thId = GetCurrentThreadId();
	return ((RETANDSRC*)g_ret_src[thId])->RET;
}
LPVOID WINAPI getSrc()
{
	DWORD thId = GetCurrentThreadId();
	return ((RETANDSRC*)g_ret_src[thId])->SRC;
}





void _declspec(naked)   myHookFunc()
{

	//NOW STACK :
	//src address + 5
	//ret
	//parm 2
	//parm 1
	//parm 0
	__asm
	{
		//CALCUALTE SRC
		POP EAX
		SUB EAX, 5

		//SAVE RET AND SRC
		PUSH EAX
		CALL setRet_Src

		/*
		NOW STACK:

			PARM 2
			PARM 1
			PARM 0
		*/

		//=============================================================



		/*
		===============================================================

		We can read the param.... in this Code
		PARAM0:[ESP]
		PARAM1:[ESP+4]
		PARAM2:[ESP+8]

		===============================================================
		*/

		//Your Code
		__asm
		{
			MOV EAX, [ESP]
			MOV EAX, [ESP + 4]
			MOV EAX, [ESP + 8]

			MOV  EAX,g_dstFunc
			CALL EAX
		}
		//You Code



		//=============================================================
		PUSH 10			//param:size

			CALL getSrc
			PUSH EAX
			CALL FindFunc
			PUSH EAX		//param:src

			CALL getSrc		//param:dst
			PUSH EAX

			CALL memcpy
			ADD ESP, 12

			CALL getEip
			ADD EAX, 11      //3 bytes  11 = 3+1+5+2
			PUSH EAX		//1 bytes
			CALL getSrc		//5 bytes
			JMP EAX			//2 bytes
			//Need RET To THIS

			/*
			STACK:
				NONE
			*/

			//Save Eax
			PUSH EAX

			CALL getSrc
			PUSH EAX
			CALL FindFunc2
			PUSH 10			//param:size

			PUSH EAX		//param:src
			CALL getSrc
			PUSH EAX		//param:dest
			CALL memcpy
			ADD ESP, 12

			/*
			STACK:
				NONE
			*/
			CALL getRet
			PUSH EAX


			MOV EAX, [ESP + 4]
			MOV EBX, [ESP]
			ADD ESP, 4
			MOV[ESP], EBX
			ret
	}

}

void SetDstFunc(LPVOID addr)
{
	g_dstFunc = addr;
}

void HookFunc(LPVOID startFunc)
{
	DWORD destAddress = DWORD(myHookFunc);
	DWORD starAddress = DWORD(startFunc);
	BYTE tmp[sizeof(JmpshellCode)];

	if (FindFunc(startFunc) == nullptr)
	{
		//Change Memory Protect
		DWORD OldPro = 0;
		VirtualProtect(startFunc, sizeof(JmpshellCode), PAGE_EXECUTE_READWRITE, &OldPro);

		LPVOID newMemcode = AddFunc(startFunc);

		//Save Src opCode
		memcpy(LPVOID(newMemcode), startFunc, sizeof(JmpshellCode));

		//Copy Jmp opCode
		memcpy(tmp, JmpshellCode, sizeof(JmpshellCode));

		//Change JMP Address
		PBYTE dstAds = PBYTE(tmp) + 6;
		(*((DWORD*)(dstAds))) = DWORD(myHookFunc) - DWORD(DWORD(startFunc) + 5) - 5;

		//Save the changed Opcode
		PBYTE dstOp = g_funcDstOpcdes[g_funcNums - 1];
		memcpy(dstOp, tmp, (sizeof(JmpshellCode)));

		//Change function header
		memcpy(startFunc, dstOp, sizeof(JmpshellCode));

	}
}