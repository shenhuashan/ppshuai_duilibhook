#include <windows.h>
#include "ADEASM.h"
#include "NAKEDHOOK.h"

#define NAKED __declspec(naked)
void NAKED Stub()
{
	__asm
	{
		pop eax
		push OLD_CODE
		push eax///���ʹ�������ļĴ������ܻ�ʧ�ܣ�һ������eax���淵�����ݣ�����������������
		_emit 0xE9
			nop
			nop
			nop
			nop
			nop
OLD_CODE:
			_emit 0xCC
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			int 3
			ret
	}
}


BOOLEAN GetNeedRvaForHook(unsigned char * Fun,ULONG* JmpRva,ULONG* OldRva,ULONG*FuncLen)
{
	unsigned char* Old_code = 0,*Jmp_code = 0,*Point = 0;
	ULONG StubLen=GetFunctionLength((BYTE*)Fun);
	if(StubLen<5)
		return FALSE;
	*FuncLen=StubLen;
	Point=(BYTE*)Fun;
	for(ULONG i=0;i<StubLen;i++)
	{
		if(Point[i]==0xe9)
			Jmp_code=&Point[i];
		if(Point[i]==0xcc)
		{
			Old_code=&Point[i];
			break;
		}		
	}
	*JmpRva=(ULONG)Jmp_code-(ULONG)Fun;
	*OldRva=(ULONG)Old_code-(ULONG)Fun;
	return TRUE;

}

BOOL InstallInlineByAddress(PVOID FuncAddr,PVOID  NewAddr,PHOOK_INFO Info)
{
	DWORD OldPro;
	ULONG JmpRva=0,OldRva=0,StubLen=0;
	int NeedLen;
	unsigned char OPCode[6]={0};
	__try
	{
	PVOID Buffer=VirtualAlloc(NULL,StubLen+8,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if(Buffer==0)
		return  FALSE;
	if(!FuncAddr||!NewAddr)
		return FALSE;
	if(!GetNeedRvaForHook((unsigned char *)Stub,&JmpRva,&OldRva,&StubLen))
		return FALSE;
	if(Info)
	{
		Info->OldAddress=FuncAddr;
		Info->StubAddress=Buffer;
		RtlCopyMemory(Info->SavedCode,FuncAddr,16);
	}
	RtlCopyMemory(Buffer,Stub,StubLen);//��Stub�ŵ��ڴ�����,Ϊ��֧�ֶ������ hook
	OPCode[0]=0x58;
	OPCode[1]=0x68;
	*(DWORD*)&OPCode[2]=(ULONG)Buffer+OldRva;
	RtlCopyMemory(Buffer,OPCode,6);
	OPCode[0]=0xe9;
	NeedLen=GetProbLength((BYTE*)FuncAddr,5);
	RtlCopyMemory((char*)Buffer+OldRva,FuncAddr,NeedLen);//���ƽ�Ҫ�����ǵ�ָ��
	*(DWORD*)&OPCode[1]=(ULONG)FuncAddr+NeedLen-((ULONG)Buffer+OldRva+NeedLen+5);
	RtlCopyMemory((char*)Buffer+OldRva+NeedLen,OPCode,5);//д������ȥִ�е�ָ��
	*(DWORD*)&OPCode[1]=(ULONG)NewAddr-((ULONG)Buffer+JmpRva+5);
	RtlCopyMemory((char*)Buffer+JmpRva,OPCode,5);//��ת���º�����ָ��
	*(DWORD*)&OPCode[1]=(ULONG)Buffer-((ULONG)FuncAddr+5);
	if (!VirtualProtect(FuncAddr,NeedLen,PAGE_EXECUTE_READWRITE,&OldPro))
	{
		VirtualFree(Buffer,0,MEM_RELEASE);
		return FALSE;
	}
	if(Info)
	{
		RtlCopyMemory(Info->SavedOpCode,OPCode,5);
	}
	RtlCopyMemory(FuncAddr,OPCode,5);//�޸�ԭʼ������ڵ�
	VirtualProtect(FuncAddr,NeedLen,OldPro,&OldPro);
	return TRUE;
	}
	__except(1)
	{
		return FALSE;
	}
}



BOOL InstallInlineByName(char *ModuleName,char *FuncName,PVOID NewAddr,PHOOK_INFO Info)
{
	PVOID FuncAddr;
	HMODULE hModule=GetModuleHandleA(ModuleName);
	if(hModule==NULL)
		hModule=LoadLibraryA(ModuleName);
	if(hModule==0)
		return FALSE;
	FuncAddr=(PVOID)GetProcAddress(hModule,FuncName);
	if(FuncAddr==0)
		return FALSE;
	return InstallInlineByAddress(FuncAddr,NewAddr,Info);
}
VOID UnInstallInline(PHOOK_INFO Info)
{
	DWORD OldPro;
	if(!Info)
		return;
	if(VirtualProtect(Info->OldAddress,16,PAGE_EXECUTE_READWRITE,&OldPro))
	{
		RtlCopyMemory(Info->OldAddress,Info->SavedCode,16);
		VirtualFree(Info->StubAddress,0,MEM_RELEASE);
		VirtualProtect(Info->StubAddress,16,OldPro,&OldPro);
	}

}

#ifdef SUPORTFASTCALL
void NAKED FSStub()
{
	__asm
	{
		pop eax
		push edx//ѹ��ڶ�������
		push eax//��ԭʼ������ַ����ȥ
		mov edx,ecx//���õڸ�����
		mov ecx,Addr//���õ�һ������
		_emit 0xE9
		nop
		nop
		nop
		nop
		nop
Addr:
		_emit 0xCC
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		int 3
		ret;

	}
}

void NAKED FCStdStub()
{
	__asm
	{
		pop eax
			push edx//ѹ��ڶ�������
			push ecx//ѹ���һ������
			push OLD_CODE//ѹ��ԭʼ��ַ
			push eax//ѹ�뷵�ص�ַ
			_emit 0xE9
			nop
			nop
			nop
			nop
			nop
OLD_CODE:
		_emit 0xCC
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			int 3
			ret

	}
}

BOOLEAN InstallInlineForFastCall(unsigned char * OldAddr,unsigned char* NewAddr,PHOOK_INFO Info,
				 BOOLEAN CovertToStd)
{
	/*
	���һ������ָ���Ƿ�ת��stdcall
	*/
	DWORD OldPro;
	ULONG JmpRva,OldRva,StubLen;
	int NeedLen;
	unsigned char OPCode[6]={0};
	unsigned char * Func=(unsigned char*)FSStub;
	if(CovertToStd)
		Func=(unsigned char *)FCStdStub;
	if(!GetNeedRvaForHook(Func,&JmpRva,&OldRva,&StubLen))
	{
		return FALSE;
	}
	if(!OldAddr||!NewAddr)
		return FALSE;
	__try
	{
		PVOID Buffer=VirtualAlloc(NULL,StubLen+8,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
		if(Buffer==0)
			return  FALSE;
		if(Info)
		{
			Info->OldAddress=OldAddr;
			Info->StubAddress=Buffer;
			RtlCopyMemory(Info->SavedCode,OldAddr,16);
		}
		RtlCopyMemory(Buffer,Func,StubLen);//��Stub�ŵ��ڴ�����,Ϊ��֧�ֶ������ hook
		if(CovertToStd)
		{//ת��stdcall
			*(DWORD*)((char*)Buffer+4)=(ULONG)Buffer+OldRva;
		}
		else
		{//ʹ��fastcall
			*(DWORD*)((char*)Buffer+6)=(ULONG)Buffer+OldRva;
		}
		OPCode[0]=0xe9;
		NeedLen=GetProbLength((BYTE*)OldAddr,5);
		RtlCopyMemory((char*)Buffer+OldRva,OldAddr,NeedLen);//���ƽ�Ҫ�����ǵ�ָ��
		*(DWORD*)&OPCode[1]=(ULONG)OldAddr+NeedLen-((ULONG)Buffer+OldRva+NeedLen+5);
		RtlCopyMemory((char*)Buffer+OldRva+NeedLen,OPCode,5);//д������ȥִ�е�ָ��
		*(DWORD*)&OPCode[1]=(ULONG)NewAddr-((ULONG)Buffer+JmpRva+5);//��ת��ַ
		RtlCopyMemory((char*)Buffer+JmpRva,OPCode,5);//��ת���º�����ָ��
		*(DWORD*)&OPCode[1]=(ULONG)Buffer-((ULONG)OldAddr+5);
		if (!VirtualProtect(OldAddr,NeedLen,PAGE_EXECUTE_READWRITE,&OldPro))
		{
			VirtualFree(Buffer,0,MEM_RELEASE);
			return FALSE;
		}
		if(Info)
		{
			RtlCopyMemory(Info->SavedOpCode,OPCode,5);
		}
		RtlCopyMemory(OldAddr,OPCode,5);//�޸�ԭʼ������ڵ�
		VirtualProtect(OldAddr,NeedLen,OldPro,&OldPro);
		return TRUE;
	}
	__except(1)
	{
		return FALSE;
	}
}



#endif