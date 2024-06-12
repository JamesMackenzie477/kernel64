#pragma once
#include <Windows.h>
#include <winternl.h>
#include <string>

#define SystemHandleInformation (SYSTEM_INFORMATION_CLASS)0x10
#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004

namespace tools
{
	namespace windows
	{
		namespace process
		{
			struct SYSTEM_HANDLE_TABLE_ENTRY
			{
				uint32_t ProcessId;
				uint8_t ObjectTypeNumber;
				uint8_t Flags;
				uint16_t Handle;
				uint64_t Object;
				ACCESS_MASK GrantedAccess;
			};

			struct SYSTEM_HANDLE_TABLE
			{
				uint32_t HandleCount;
				SYSTEM_HANDLE_TABLE_ENTRY Handles[1];
			};

			uint64_t get_address(uint32_t pid)
			{
				ULONG length = 10000;

				uint64_t address = 0;

				auto hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);

				if (hProcess == INVALID_HANDLE_VALUE) return address;

				SYSTEM_HANDLE_TABLE * pHandles = reinterpret_cast<SYSTEM_HANDLE_TABLE *>(malloc(length));

				ZeroMemory(pHandles, length);

				while (NtQuerySystemInformation(SystemHandleInformation, pHandles, length, &length) == STATUS_INFO_LENGTH_MISMATCH)
				{

					pHandles = reinterpret_cast<SYSTEM_HANDLE_TABLE *>(realloc(pHandles, length));

					ZeroMemory(pHandles, length);
				}

				for (int i = 0; i < pHandles->HandleCount; i++)
				{

					if (pHandles->Handles[i].ProcessId == GetCurrentProcessId() && pHandles->Handles[i].Handle == reinterpret_cast<uint16_t>(hProcess))
					{
						address = pHandles->Handles[i].Object;

						break;
					}
				}

				free(pHandles);

				CloseHandle(hProcess);

				return address;
			}
		}
	}
}