#include <ntifs.h>
#include <intrin.h>
#include <ntddk.h>
#include <ntimage.h>
#define OR_DR_DRIVER_NAME "OMRI"
#define OR_DR_LINK_NAME "\\??\\" OR_DR_DRIVER_NAME
#define OR_DR_LINK_NAME_L	L"\\??\\" OR_DR_DRIVER_NAME
#define OR_DR_DEVICE_NAME "\\Device\\" OR_DR_DRIVER_NAME
#define OR_DR_DEVICE_NAME_L L"\\Device\\" OR_DR_DRIVER_NAME
PDEVICE_OBJECT gALdrDeviceObj = 0;
#define OR_DR_GET_IO_CODE(a) ((a&0x3ffc)>>2)-0x7ff
PVOID	 ALmemMapPhysicalMemory(void* PhysicalAddress, UINT64 size);
UINT64 check_hook1(PVOID target);
#define dbgPut(a,...) DbgPrintEx(0,0,a,__VA_ARGS__)
#define ALdbgPut(a,...) dbgPut("[OMRI]-->{" __FUNCTION__ "}\t/%d/:" a "\n",__LINE__,__VA_ARGS__) 
#define ALdbgPutValue(a)		 ALdbgPut(#a" %p",(PVOID)a)

NTSTATUS ALdrIoHandler(UINT64 ioCode, PVOID iData, UINT64 iSize, PVOID oData, UINT64 oSize)
{
	iData;
	iSize;
	oData;
	oSize;
	ioCode &= 0xffff;
	switch (ioCode)
	{
	case 1:
	{
		ALdbgPut("开始映射");
		auto tar_pa = MmGetPhysicalAddress(NtOpenProcess).QuadPart;
		ALdbgPutValue(tar_pa);
		auto r = ALmemMapPhysicalMemory((PVOID)tar_pa, 0x1000);
		ALdbgPutValue(r);
		**(PVOID**)oData = r;
		break;
	}
	case 2:
	{
		auto ad = check_hook1(NtOpenProcess);
		if (ad == 0)
		{
			ALdbgPut("未检测到hook");
		}
		else {
			ALdbgPut("检测到hook.跳转地址:%p", ad);
		}
		break;
	}
	default:
		break;
	}
	return 0;
}

NTSTATUS ALdrIoSecurityToHandler(PDEVICE_OBJECT, PIRP pIrp)
{
	PVOID InputData = NULL;
	ULONG InputDataLength = 0;
	PVOID OutputData = NULL;
	ULONG OutputDataLength = 0;
	ULONG IoControlCode = 0;
	PIO_STACK_LOCATION  IoStackLocation = IoGetCurrentIrpStackLocation(pIrp);  //Irp堆栈  
	//ALdbgPut("%p", IoStackLocation->Parameters.DeviceIoControl.IoControlCode);
	IoControlCode = OR_DR_GET_IO_CODE(IoStackLocation->Parameters.DeviceIoControl.IoControlCode);
	InputData = pIrp->AssociatedIrp.SystemBuffer;
	OutputData = pIrp->AssociatedIrp.SystemBuffer;
	InputDataLength = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
	OutputDataLength = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	pIrp->IoStatus.Information = ALdrIoHandler(IoControlCode, InputData, InputDataLength, OutputData, OutputDataLength);
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);  //将Irp返回给Io管理器
	return 0;
}
NTSTATUS ALdrDelIo(PDRIVER_OBJECT )
{
	//创建设备对象
	UNICODE_STRING linkName;
	RtlInitUnicodeString(&linkName, OR_DR_LINK_NAME_L);
	IoDeleteSymbolicLink(&linkName);//可能有同名的符号链接因此先删除
	if (gALdrDeviceObj)
		IoDeleteDevice(gALdrDeviceObj);
	return 0;

}
NTSTATUS ALdrSetIo(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING deviceName;
	//创建设备对象
	RtlInitUnicodeString(&deviceName, OR_DR_DEVICE_NAME_L);
	IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &gALdrDeviceObj);
	//通信方式
	if (gALdrDeviceObj)
		gALdrDeviceObj->Flags |= DO_BUFFERED_IO;
	//创建符号链接

	UNICODE_STRING linkName;
	RtlInitUnicodeString(&linkName, OR_DR_LINK_NAME_L);
	IoDeleteSymbolicLink(&linkName);//可能有同名的符号链接因此先删除
	if (STATUS_SUCCESS != IoCreateSymbolicLink(&linkName, &deviceName))
	{
		if (gALdrDeviceObj)
			IoDeleteDevice(gALdrDeviceObj);
		//IoDeleteSymbolicLink(&设备别名);
		return 1;
	}
	//打开
	DriverObject->MajorFunction[IRP_MJ_CREATE] = ALdrIoSecurityToHandler;
	//关闭
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = ALdrIoSecurityToHandler;
	//交互
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ALdrIoSecurityToHandler;
	return STATUS_SUCCESS;
}
