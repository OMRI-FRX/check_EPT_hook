#include <ntddk.h>
#include <intrin.h>
#include "idt.h"
//#include <ntifs.h>
#define dbgPut(a,...) DbgPrintEx(0,0,a,__VA_ARGS__)
#define ALdbgPut(a,...) dbgPut("[OMRI]-->{" __FUNCTION__ "}\t/%d/:" a "\n",__LINE__,__VA_ARGS__) 
#define ALdbgPutValue(a)		 ALdbgPut(#a" %p",(PVOID)a)
extern "C" UINT64 Glitch(PVOID add, UINT64 inst_count);
unsigned int get_inst_sz(PVOID add, UINT32 is64 = 1);
int filter1(unsigned int code, struct _EXCEPTION_POINTERS* ep, PVOID& hook_handler_add, PVOID target, int inst_sz)
{
	code;
	ALdbgPutValue(ep->ExceptionRecord->ExceptionAddress);
	if (ep->ExceptionRecord->ExceptionAddress == target)
	{
		ALdbgPutValue(ep->ContextRecord->EFlags);
		ep->ContextRecord->EFlags |= 0x100;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else
	{
		if ((UINT64)ep->ExceptionRecord->ExceptionAddress != (UINT64)target + inst_sz)
		{
			hook_handler_add = ep->ExceptionRecord->ExceptionAddress;
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}
}
//UINT64 check_hook1_(PVOID target)
//{
//	PVOID hook_handler_add = 0;
//	int inst_sz = get_inst_sz(target);
//	__try {
//		//ALdbgPut();
//		Glitch(target);
//		//ALdbgPut();
//		return 1;
//	}
//	__except (filter1(GetExceptionCode(), GetExceptionInformation(), hook_handler_add, target, inst_sz))
//	{
//		//ALdbgPut("1");
//		return (UINT64)hook_handler_add;
//	}
//}
#pragma pack(1)
typedef struct {
	UINT16 limit; //
	UINT32 base;  //
} GDTR32_t, LDTR32_t, IDTR32_t;
static_assert (sizeof(GDTR32_t) == 6, "");

typedef struct {
	UINT16 limit; //
	UINT64 base;  //
} GDTR64_t, LDTR64_t, IDTR64_t;
static_assert (sizeof(GDTR64_t) == 10, "");
#pragma pack()
#define ALdbgKill(a,b) KeBugCheckEx(('or'<<16)+__LINE__,(ULONG_PTR)(a),(ULONG_PTR)(b),(ULONG_PTR)__FUNCTION__,(ULONG_PTR)__LINE__)

UINT64 check_hook1(PVOID target)
{
	IDTR64_t idtr = { 0 };
	__sidt(&idtr);
	auto idtbs = idtr.base;
	PHYSICAL_ADDRESS a = { 0 };
	a.QuadPart = (UINT64)idtbs;
	//auto idt_va = MmGetVirtualForPhysical(a);
	auto idt_va = idtbs;
	if (idt_va == 0)
	{
		return 0;
	}
	//auto olde = ((IDTE*)idt_va)[1];

	int inst_sz = get_inst_sz(target);
	auto next_inst = (UINT64)target + inst_sz;
	auto nn_inst = next_inst + get_inst_sz((PVOID)next_inst);

	auto n_idt = (IDTE*)ExAllocatePoolWithTag(NonPagedPool, 0x1000, '1112');
	if (n_idt == 0)
	{
		ALdbgPut("申请内存失败");
		return 0;
	}
	memcpy(n_idt, (PVOID)idt_va, 0x1000);
	IDTR64_t n_idtr = idtr;
	n_idtr.base = (UINT64)n_idt;

	/*auto o_cr0 = __readcr0();
	auto n_cro = o_cr0 & ~0x10000;
	__writecr0(n_cro);*/


	ALvmSetIdteFunAdd(&((IDTE*)n_idt)[1], (UINT64)&interrupt_handler_01_DB);	 //接管DB

	_disable();																	 //关闭中断

	__lidt(&n_idtr);															 //设置新的IDT

	auto r = Glitch(target, (UINT64)2);											 //开始探测

	__lidt(&idtr);																 //探测完毕改回原IDT

	_enable();																	 //启用中断
	ExFreePoolWithTag(n_idt, '1112');											 //释放构造的IDT

	if (r != nn_inst)															 //对比指令地址是否符合预期
		return r;
	else
		return 0;

}

NTSTATUS ALdrDelIo(PDRIVER_OBJECT DriverObject);

void driver_unload(PDRIVER_OBJECT a) {

	ALdrDelIo(a);
	DbgPrint("[hv] Devirtualized the system.\n");
	DbgPrint("[hv] Driver unloaded.\n");
}
NTSTATUS ALdrSetIo(PDRIVER_OBJECT DriverObject);

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT drv, PUNICODE_STRING)
{
	if (drv)
		drv->DriverUnload = driver_unload;

	/*OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
	CLIENT_ID ClientId = { 0 };
	HANDLE hd = (HANDLE)0x123;
	auto r = NtOpenProcess(&hd, 0, &ObjectAttributes, &ClientId);
	if (r != 112233)
	{
		ALdbgPut("没hook");
		return 0;
	}*/
	ALdrSetIo(drv);

	
	return  0;
}