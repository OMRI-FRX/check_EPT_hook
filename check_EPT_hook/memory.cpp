#include <ntdef.h>
#include <ntifs.h>
#include <intrin.h>
#include <ntddk.h>
#include <ntimage.h>
PVOID	 ALmemMapPhysicalMemory(void* PhysicalAddress, UINT64 size)
{

	//打开内存驱动句柄
	NTSTATUS status;
	UNICODE_STRING physicalMemoryString;
	OBJECT_ATTRIBUTES attributes;
	HANDLE hPhysicalhandle = 0;
	WCHAR physicalMemoryName[] = L"\\Device\\PhysicalMemory";
	RtlInitUnicodeString(&physicalMemoryString, physicalMemoryName);
	InitializeObjectAttributes(&attributes, &physicalMemoryString, 0, NULL, NULL);
	status = ZwOpenSection(&hPhysicalhandle, SECTION_MAP_READ | SECTION_MAP_WRITE, &attributes);
	if (status != STATUS_SUCCESS)
	{
		DbgPrint("打开内存失败:%X", status);
		return 0;
	}
	//映射物理地址
	PHYSICAL_ADDRESS physicalBase;
	physicalBase.QuadPart = (UINT64)PhysicalAddress & (~0xfff);
	PVOID virtualBase = 0;
	UINT64 exactAddress = (UINT64)PhysicalAddress & 0xfff;
	SIZE_T mappingSize = exactAddress + size;
	mappingSize += mappingSize % 0x1000 ? 0x1000 : 0;
	mappingSize &= (~0xfff);
	status = ZwMapViewOfSection(
		hPhysicalhandle,
		NtCurrentProcess(),
		(PVOID*)&virtualBase,
		0,
		mappingSize,
		&physicalBase,
		&mappingSize,
		ViewShare,
		MEM_TOP_DOWN,
		PAGE_EXECUTE_READWRITE);
	if (status != 0)
	{
		DbgPrint("映射物理地址失败:%x\n", status);
		return 0;
	}
	if (hPhysicalhandle != NULL)
	{
		ZwClose(hPhysicalhandle);
	}
	return (PVOID)((UINT64)virtualBase + exactAddress);
}