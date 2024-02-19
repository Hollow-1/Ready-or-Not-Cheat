#pragma once
#include "Settings.h"
#include "vmmdll.h"
#include <iostream>
#include <unordered_map>
#include <deque>
#include <mutex>
#include <Windows.h>
#include <TlHelp32.h>

#define Memory MemoryManager::Get()

/*
* 这里实现一个简单的缓存管理
* 每次从内存读取之前，先判断有没有缓存，如果没有，读取一个内存页（4K）放进缓存
* 如果缓存的页达到最大数量，先移除最旧的缓存再添加
* 如果要读取的数据跨越页边距，则读取多个页，然后拼接数据
* 清除缓存的时候，将已经分配的内存块放进 freeList 以便复用
*/
class MemoryManager : public Singleton<MemoryManager> {
private:
	const DWORD PAGE_SIZE = 0x1000;  //4096
	const DWORD MAX_CACHE_SIZE = 0x500; //1280 * 4K 最大缓存5MB

	std::unordered_map<ULONG_PTR, PBYTE> cache;
	std::deque<ULONG_PTR> queue;
	std::list<PBYTE> freeList;
	std::mutex threadLock;

//#define _DMA_    //可以使用 DMA ，也可以使用 windows 提供的函数

#ifdef _DMA_

//使用 DMA 操作进程和内存
private:
	VMM_HANDLE hVMM = NULL; //VMMDLL Handle

	bool InitPrivate()
	{
        //如果使用的是 DMA 板卡，需要把 vmware 改为 fpga
		char* args[] = { (char*)"",(char*)"-device",(char*)"vmware" };
		hVMM = VMMDLL_Initialize(3, args);

		if (hVMM == NULL)
		{
			cout << "DMA: Memory Initialize failed" << endl;
			return false;
		}
		GameVars.processID = GetProcessID(GameVars.targetProcess);
		GameVars.moduleBase = GetBaseAddress(GameVars.processID, GameVars.targetProcess);

		return true;
	}

	DWORD GetProcessID(const char* processName)
	{
		SIZE_T pcPIDs = 0;
		VMMDLL_PidList(hVMM, NULL, &pcPIDs);
		DWORD* pPIDs = (DWORD*)malloc(pcPIDs * 4);
		VMMDLL_PidList(hVMM, pPIDs, &pcPIDs);

		DWORD pid = 0;
		if (pPIDs)
		{
			for (DWORD i = 0; i < pcPIDs; i++)
			{
				LPSTR path = VMMDLL_ProcessGetInformationString(hVMM, pPIDs[i], VMMDLL_PROCESS_INFORMATION_OPT_STRING_PATH_USER_IMAGE);
				if (!path) { continue; }
				//std::cout << path << std::endl;

				char* lastSlash = strrchr(path, '\\');
				char* exeName = lastSlash + 1;

				if (strcmp(exeName, processName) == 0)
				{
					pid = pPIDs[i];
					VMMDLL_MemFree(path);
					break;
				}
				VMMDLL_MemFree(path);
			}
		}

		free(pPIDs);
		return pid;
	}

	ULONG_PTR GetBaseAddress(DWORD pid, const char* targetProcess) {
		return VMMDLL_ProcessGetModuleBaseU(hVMM, pid, (LPSTR)targetProcess);

	}

	PBYTE ReadPage(ULONG_PTR address)
	{
		ULONG64 flags = VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOCACHEPUT | VMMDLL_FLAG_NO_PREDICTIVE_READ;
		PBYTE buffer = GetPageBuffer();
		DWORD bytesRead = 0;

		bool result = VMMDLL_MemReadEx(hVMM, GameVars.processID, address, buffer, PAGE_SIZE, &bytesRead, flags);
		if (!result || bytesRead != PAGE_SIZE)
		{
			std::cout << "Read Page Failed: p=" << GameVars.processID <<
				", addr=0x" << std::hex << address <<
				", size=0x" << std::hex << PAGE_SIZE <<
				", bytesRead=0x" << std::hex << bytesRead << std::endl;

			freeList.push_back(buffer);
			return NULL;
		}
		return buffer;
	}
public:
	template<typename T> void Write(ULONG_PTR address, T& value) {
		VMMDLL_MemWrite(hVMM, GameVars.processID, address, &value, sizeof(T));
	}


	~MemoryManager() {
		ClearCache();
		for (auto& ptr : freeList) {
			delete[] ptr; // 清空自由列表中的内存块
		}
		VMMDLL_CloseAll();
	}

#else
// windows 提供的用于内存，进程操作的函数，适用于没有反作弊保护的游戏

private:
	HANDLE handle = NULL;

	bool InitPrivate()
	{
		GameVars.processID = GetProcessID(GameVars.targetProcess);
		GameVars.moduleBase = GetBaseAddress(GameVars.processID, GameVars.targetProcess);
		handle = ::OpenProcess(PROCESS_ALL_ACCESS, NULL, GameVars.processID);//Use Kernel if u have so much problem ;)
		if (handle == NULL) {
			cout << "Memory Initialize failed" << endl;
			return false;
		}
		return true;
	}
	//窄字符转换为宽字符，使用完毕需要手动释放  delete[] return
	wchar_t* ToWideChar(const char* narrowString)
	{
		if (narrowString == NULL) {
			return NULL;
		}
		size_t size = strlen(narrowString) + 1;
		size_t convertedSize = 0;
		wchar_t* wideString = new wchar_t[size];
		mbstowcs_s(&convertedSize, wideString, size, narrowString, size);
		return wideString;
	}

	DWORD GetProcessID(const char* processName)
	{
		wchar_t* wideName = ToWideChar(processName);

		DWORD pid = 0;
		DWORD threadCount = 0;
		HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hSnap, &pe);
		while (Process32Next(hSnap, &pe)) {
			if (wcscmp(pe.szExeFile, wideName) == 0) {
				if (pe.cntThreads > threadCount) {
					threadCount = pe.cntThreads;
					pid = pe.th32ProcessID;
				}
			}
		}
		::CloseHandle(hSnap);
		delete[] wideName;
		return pid;
	}
	ULONG_PTR GetBaseAddress(DWORD pid, const char* targetProcess)
	{
		wchar_t* wideName = ToWideChar(targetProcess);
		ULONG_PTR base = NULL;

		HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
		if (hSnap != INVALID_HANDLE_VALUE) {
			MODULEENTRY32 modEntry;
			modEntry.dwSize = sizeof(modEntry);
			if (Module32First(hSnap, &modEntry)) {
				do {
					if (wcscmp(modEntry.szModule, wideName) == 0) {
						base = (ULONG_PTR)modEntry.modBaseAddr;
						break;
					}
				} while (Module32Next(hSnap, &modEntry));
			}
		}
		::CloseHandle(hSnap);
		delete[] wideName;
		return base;
	}
	PBYTE ReadPage(ULONG_PTR address)
	{
		PBYTE buffer = GetPageBuffer();
		size_t bytesRead = 0;
		bool result = ::ReadProcessMemory(handle, (LPCVOID)address, buffer, PAGE_SIZE, &bytesRead);

		if (!result || bytesRead != PAGE_SIZE)
		{
			std::cout << "Read Page Failed: p=" << GameVars.processID <<
				", addr=0x" << std::hex << address <<
				", size=0x" << std::hex << PAGE_SIZE <<
				", bytesRead=0x" << std::hex << bytesRead << std::endl;

			freeList.push_back(buffer);
			return NULL;
		}
		return buffer;
	}

public:

	template<typename T> void Write(ULONG_PTR address, T& value) {
		::WriteProcessMemory(handle, (LPVOID)address, &value, sizeof(T), NULL);
	}

	~MemoryManager() {
		ClearCache();
		for (auto& ptr : freeList) {
			delete[] ptr; // 清空自由列表中的内存块
		}
		::CloseHandle(handle);
	}

#endif


public:

	/* return	true  : success
				false : failed */
	bool Init()
	{
		if (InitPrivate() == false)
		{
			return false;
		}

		cout << "ProcessID Found at " << GameVars.processID << endl;
		cout << "Base Address Found at 0x" << hex << GameVars.moduleBase << dec << endl;
		if (GameVars.processID == 0 || GameVars.moduleBase == 0)
		{
			return false;
		}
		return true;
	}

	void ClearCache() {
		std::lock_guard<std::mutex> lock(threadLock);
		for (auto& pair : cache) {
			freeList.push_back(pair.second); // 将缓存的内存块加入自由列表
		}
		cache.clear(); // 清除缓存中的数据
		queue.clear(); // 清空队列
	}

	bool Read(ULONG_PTR address, PBYTE buffer, DWORD size, bool noCache = false) {
		ULONG_PTR start_page = address >> 12;				// 除 4096， 得到页编号
		ULONG_PTR end_page = (address + size - 1) >> 12;

		ULONG_PTR start_offset = address % PAGE_SIZE;			//第一个页面的开始偏移
		ULONG_PTR end_offset = start_offset + size - ((end_page - start_page) * PAGE_SIZE);//最后一个页面的结束偏移
		ULONG_PTR data_index = 0;

		{
			//保证线程安全
			std::lock_guard<std::mutex> lock(threadLock);
			for (ULONG_PTR i = start_page; i <= end_page; ++i) {

				PBYTE page_data = GetPage(i, noCache);
				if (page_data == NULL) {
					memset(buffer, 0, size);
					return false;
				}

				//拼接数据
				ULONG_PTR from = (i == start_page) ? start_offset : 0;
				ULONG_PTR to = (i == end_page) ? end_offset : PAGE_SIZE;
				ULONG_PTR copy_size = to - from;
				std::memcpy(buffer + data_index, page_data + from, copy_size);
				data_index += copy_size;
			}
		}

		return true;
	}
	/*
	template<typename T> T Read(ULONG_PTR address) {
		T buffer;
		Read(address, &buffer, sizeof(T));
		return buffer;
	}
	*/



private:
	//获取一个页，用来从中读取数据
	PBYTE GetPage(ULONG_PTR pageIndex, bool noCache)
	{
		//如果缓存中没有
		if (cache.find(pageIndex) == cache.end()) {

			PBYTE page_data = ReadPage(pageIndex << 12);
			if (page_data == NULL) {
				return NULL;
			}
			cache[pageIndex] = page_data;
			queue.push_back(pageIndex);
			return page_data;
		}
		//命中缓存，而且可以使用缓存
		if (noCache == false) {
			return cache[pageIndex];
		}

		//命中缓存，但不能使用缓存
		PBYTE new_page = ReadPage(pageIndex << 12);
		if (new_page == NULL) {
			return NULL;
		}
		PBYTE old_page = cache[pageIndex];
		cache[pageIndex] = new_page;
		freeList.push_back(old_page);

		//将这个索引移动到队列的最后
		auto it = std::find(queue.begin(), queue.end(), pageIndex);
		if (it != queue.end()) {
			queue.erase(it);
			queue.push_back(pageIndex);
		}
		return new_page;
	}

	//获取一个可用的页缓冲区，用来存储页数据
	PBYTE GetPageBuffer()
	{
		//检查缓存大小，如果达到最大缓存，移除最旧的缓存
		while (cache.size() >= MAX_CACHE_SIZE)
		{
			ULONG_PTR key = queue.front();
			PBYTE page = cache[key];

			queue.pop_front();
			cache.erase(key);
			freeList.push_back(page);
		}

		PBYTE buffer = NULL;
		if (freeList.empty()) {
			buffer = new byte[PAGE_SIZE]; // 如果自由列表为空，则分配新内存
		}
		else {
			buffer = freeList.front(); // 获取列表中的第一个内存块
			freeList.pop_front(); // 移除列表中的第一个内存块
		}
		return buffer;
	}

};

template<typename T> T Read(ULONG_PTR address) {
	T buffer;
	//如果地址特别小，可能是 空指针 + 偏移
	if (address < 0x10000)
	{
		memset(&buffer, 0, sizeof(T));
		cout << "Read failed, address is too small => 0x" << std::hex << address << endl;
		return buffer;
	}

	bool result = Memory.Read(address, (PBYTE)&buffer, sizeof(T));
	if (!result)
	{
		cout << "Read failed 0x" << std::hex << address << endl;
	}
	return buffer;
}

string ReadString(ULONG_PTR address, uint32 size) {
	string name(size, '\0');
	Memory.Read(address, (PBYTE)name.data(), size * sizeof(char));
	name.shrink_to_fit();
	return name;
}