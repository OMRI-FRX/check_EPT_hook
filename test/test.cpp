// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>
#define dbgPut(a,...) printf(a,__VA_ARGS__)
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
LONG veh_handler(PEXCEPTION_POINTERS pExcepInfo)
{
	auto ExceptionAddress = (UINT64)pExcepInfo->ExceptionRecord->ExceptionAddress;
	//pExcepInfo->ContextRecord->Rip;
	ALdbgPut("已调用异常处理 %p", ExceptionAddress);
	auto frame = pExcepInfo->ContextRecord;
	if (frame->Rip == frame->Rcx)//执行到目标地址
	{
		frame->Rdx--;
		pExcepInfo->ContextRecord->EFlags |= 0x100;
		return EXCEPTION_CONTINUE_EXECUTION;		   //继续执行
	}
	else
	{
		//ALdbgKill(frame->rip, frame->rdx);

		if (frame->Rdx != 0)
		{
			frame->Rdx--;
			pExcepInfo->ContextRecord->EFlags |= 0x100;
			return EXCEPTION_CONTINUE_EXECUTION;		   //继续执行
		}
		else
		{
			//ALdbgKill(frame->Rip, frame->R8);

			frame->Rax = frame->Rip;
			frame->Rip = frame->R8;			   //不再继续执行
			frame->Rsp = frame->R9;
			return EXCEPTION_CONTINUE_EXECUTION;		   
		}

	}
}

UINT64 check_hook1_(PVOID target)
{
	int inst_sz = get_inst_sz(target);
	auto next_inst = (UINT64)target + inst_sz;
	auto nn_inst = next_inst + get_inst_sz((PVOID)next_inst);

	AddVectoredExceptionHandler(1, veh_handler);
	//ALdbgPut();
	auto r = Glitch(target, 2);
	if (r != nn_inst)															 //对比指令地址是否符合预期
		return r;
	else
		return 0;
}
#define OR_DR_DRIVER_NAME "OMRI"
#define OR_DR_LINK_NAME "\\??\\" OR_DR_DRIVER_NAME
#define USER ('or'<<16)
#define OR_DR_MAKE_IO_CODE_U(a)	 a+USER 
UINT64 ALdrDriverIO(HANDLE hd, UINT64 ioCode, PVOID in, UINT64 iSize, PVOID out, UINT64 oSize)
{
	DWORD size;
	DeviceIoControl(hd, OR_DR_MAKE_IO_CODE_U(CTL_CODE(FILE_DEVICE_UNKNOWN, ioCode + 0x7ff, METHOD_BUFFERED, FILE_ANY_ACCESS)), in, iSize, out, oSize, &size, NULL);
	return size;
}
int main()
{
	auto DriverHandle = CreateFileA(OR_DR_LINK_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == DriverHandle)
	{
		ALdbgPut("驱动未加载");
		printf("Create File Error %d \n", GetLastError());
		while (1);
		return 0;
	}
	PVOID add = 0;
	PVOID* add_p = &add;
	ALdrDriverIO(DriverHandle, 1, &add_p, 8, &add_p, 8);
	ALdbgPutValue(add);
	ALdbgPut("开始探测");
	auto r = check_hook1_(add);
	if (r != 0)
	{
		ALdbgPut("发现hook :%p", r);
	}
	else {
		ALdbgPut("未发现hook");
	}

    std::cout << "Hello World!\n";
	while (true)
	{

	}
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
