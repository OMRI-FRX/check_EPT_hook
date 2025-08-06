#include "idt.h"

void ALvmSetIdteFunAdd(IDTE* idte_, UINT64 FunctionAdd)
{
	/*idte_->interrupt_stack_table = 0;
	idte_->segment_selector = ALvmGetCs();
	idte_->must_be_zero_0 = 0;
	idte_->type = SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE;
	idte_->must_be_zero_1 = 0;
	idte_->descriptor_privilege_level = 0;
	idte_->present = 1;
	idte_->reserved = 0;*/
	IDTE* pIdte = (IDTE*)idte_;
	pIdte->offset_low = (FunctionAdd >> 0) & 0xFFFF;
	pIdte->offset_middle = (FunctionAdd >> 16) & 0xFFFF;
	pIdte->offset_high = (FunctionAdd >> 32) & 0xFFFFFFFF;
	//idteAdd.add1 = pIdte->offset_low ;
	//idteAdd.add2 = pIdte->offset_middle ;
	//idteAdd.add3 = pIdte->offset_high;
	//ALdbgPutValue(idteAdd.add);
}
#define ALdbgKill(a,b) KeBugCheckEx(('or'<<16)+__LINE__,(ULONG_PTR)(a),(ULONG_PTR)(b),(ULONG_PTR)__FUNCTION__,(ULONG_PTR)__LINE__)
extern "C" UINT64 ALvmIdtHandler(trap_frame * frame)
{
	if (frame->rip == frame->rcx)//执行到目标地址
	{
		frame->rdx--;
		return 0;//继续执行
	}
	else
	{
		//ALdbgKill(frame->rip, frame->rdx);

		if (frame->rdx != 0)
		{
			frame->rdx--;
			return 0;  //继续执行
		}
		else
		{
			//ALdbgKill(frame->rip, frame->r8);

			frame->rax = frame->rip;
			frame->rip = frame->r8;			   //不再继续执行
			frame->rsp = frame->r9;
			frame->rflags &= ~0x100;		   //取消单步断点
			return 0;
		}

	}
}