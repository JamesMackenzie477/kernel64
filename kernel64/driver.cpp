#include "driver.h"

// Loads the driver.
tools::windows::kernel::driver::driver()
{
	load();
}

// Unloads the driver.
tools::windows::kernel::driver::~driver()
{
	unload();
}

// Loads the driver.
bool tools::windows::kernel::driver::load()
{
	WCHAR path[MAX_PATH];
	DWORD bytes;
	SERVICE_STATUS status;

	// Writes the driver to the disk (overwrites existing).

	if (!ExpandEnvironmentStringsW(DRIVER_PATH, path, sizeof(path))) return false;

	auto hFile = CreateFileW(path, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile == INVALID_HANDLE_VALUE) return false;

	auto result = WriteFile(hFile, driver_bytes, sizeof(driver_bytes), &bytes, nullptr);

	CloseHandle(hFile);

	if (!result) return false;

	// Opens the service manager.

	auto hManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE | SC_MANAGER_CONNECT);

	if (!hManager) return false;

	// Removes the existing service.

	auto hExists = OpenServiceW(hManager, SERVICE_NAME, SERVICE_STOP | DELETE);

	if (hExists)
	{
		auto result = (!ControlService(hExists, SERVICE_CONTROL_STOP, &status) && GetLastError() != ERROR_SERVICE_NOT_ACTIVE) || (!DeleteService(hExists) && GetLastError() != ERROR_SERVICE_MARKED_FOR_DELETE);

		CloseServiceHandle(hExists);

		if (result) return false;
	}

	// Creates the driver service.

	service = CreateServiceW(hManager, SERVICE_NAME, SERVICE_NAME, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, path, nullptr, nullptr, nullptr, nullptr, nullptr);

	CloseServiceHandle(hManager);

	if (!service && GetLastError() != ERROR_SERVICE_EXISTS) return false;

	// Starts the driver service.

	if (!StartServiceW(service, 0, nullptr) && GetLastError() != ERROR_SERVICE_ALREADY_RUNNING)
	{
		DeleteService(service);

		CloseServiceHandle(service);

		return false;
	}

	// Opens the driver.

	handle = CreateFileW(SYMBOLIC_LINK, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (handle == INVALID_HANDLE_VALUE)
	{
		ControlService(service, SERVICE_CONTROL_STOP, &status);

		DeleteService(service);

		CloseServiceHandle(service);

		return false;
	}

	return true;
}

// Unloads the driver.
bool tools::windows::kernel::driver::unload() const
{
	SERVICE_STATUS status;

	// Closes the driver.

	if (!CloseHandle(handle)) return false;

	// Stops the driver service.

	if (!ControlService(service, SERVICE_CONTROL_STOP, &status)) return false;

	// Deletes the driver service.

	if (!DeleteService(service)) return false;

	// Closes the service.

	if (!CloseServiceHandle(service)) return false;

	return true;
}

// Returns the driver instance.
tools::windows::kernel::driver& tools::windows::kernel::driver::get_instance()
{
	static driver instance;
	return instance;
}

// Reads the specified amount from the given memory address.
bool tools::windows::kernel::driver::read(uint64_t addr, void * buff, uint32_t len) const
{
	read_memory_params params;
	DWORD bytes;

	params.address = addr;
	params.length = len;

	if (!DeviceIoControl(handle, IOCTL_READ_MEMORY, &params, sizeof(params), buff, len, &bytes, NULL)) return false;

	return true;
}

// Writes the given value to the specified memory address.
bool tools::windows::kernel::driver::write(uint64_t addr, uint64_t val) const
{
	write_memory_params params;
	DWORD bytes;

	params.address = addr - 96;
	params.option = 28;
	params.value = val;

	if (!DeviceIoControl(handle, IOCTL_WRITE_MEMORY, &params, sizeof(params), nullptr, 0, &bytes, NULL)) return false;

	return true;
}