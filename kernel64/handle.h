#pragma once
#include <Windows.h>
#include "driver.h"
#include "process.h"

#define OBJECT_TABLE_OFFSET 0x200
#define TABLE_DEPTH_BITMASK 3
#define HANDLE_BITMASK 0xFFFFFFFFFFFFFFFC
#define OBJECT_ADDRESS_BITMASK 0xFFFFFFFFFFFFFFF8
#define OBJECT_HEADER_SIZE 0x30

namespace tools
{
	namespace windows
	{
		namespace handle
		{
			// Returns the address of the handle entry.
			uint64_t get_address(uint32_t pid, HANDLE handle)
			{
				uint64_t objTable, tableBase;

				// Gets the target processes object address.

				auto objAddr = tools::windows::process::get_address(pid);

				if (!objAddr) return 0;

				// Reads the object table address and table base address.

				auto& drv = tools::windows::kernel::driver::get_instance();

				if (!drv.read(objAddr + OBJECT_TABLE_OFFSET, &objTable, sizeof(objTable))) return 0;

				if (!drv.read(objTable, &tableBase, sizeof(tableBase))) return 0;

				// Ensures that there is only one table and removes the table depth bits from the table base address.

				if (tableBase & TABLE_DEPTH_BITMASK) return 0;

				tableBase = tableBase & ~TABLE_DEPTH_BITMASK;

				// Calculates and returns the handle table entry address.

				return tableBase + (reinterpret_cast<uint64_t>(handle) & HANDLE_BITMASK) * 4;
			}
		}

		namespace wrappers
		{
			class handle
			{
			private:

				HANDLE han;

			public:

				handle(HANDLE han)
					: han(han)
				{
				}

				// Gets the address of the object owned by the handle.
				uint64_t get_object_address()
				{
					return 0;
				}

				// Sets the address of the object owned by the handle.
				bool set_object_address(uint64_t addr)
				{
					return 0;
				}

				// Gets the handle access rights.
				uint32_t get_access_rights()
				{
					return 0;
				}

				// Sets the handle access rights.
				bool set_access_rights(uint32_t acc)
				{
					return 0;
				}

				// Returns the raw handle.
				HANDLE get_raw()
				{
					return han;
				}

				// Closes the handle.
				bool close()
				{
					return CloseHandle(han);
				}

				~handle()
				{
					close();
				}
			};
		};
	}
}