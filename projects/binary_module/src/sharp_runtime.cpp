#include "sharp_runtime.h"
#include "os_definions.h"

#include <utility>
#include <system_error>
#include "utils/utils_validating.h"

#ifdef WIN32
#define CORE_CLR_LIBRARY "coreclr.dll"
#define FS_SEPARATOR "\\"
#define PATH_DELIMITER ";"
#endif

extern "C" {
	typedef void* (__stdcall *ModuleInitializePtr)(const char* modulesPath, const char* baseLibsPath, const char* apiLibsPath, const char* apiLibName);
	typedef int (__stdcall *ModuleCompileInlinePtr)(const char* code);
}

DWORD Win32FromHResult(HRESULT hr);

gm_sharp::Runtime::Runtime(std::string sRootPath, std::string sSharedLibsPath, std::string sLibsPath, std::string sMainLibName, std::string sModulesLookupPath)
	: sRootRuntimePath(std::move(sRootPath)), sSharedLibsPath(std::move(sSharedLibsPath)), sLibsPath(std::move(sLibsPath)), 
	  sMainLibName(std::move(sMainLibName)), sModulesLookupPath(std::move(sModulesLookupPath))
{

}

bool gm_sharp::Runtime::Initialize(long* err) {
	char libraryPath[MAX_PATH];
	snprintf(libraryPath, sizeof(libraryPath), "%s" FS_SEPARATOR CORE_CLR_LIBRARY, sRootRuntimePath.c_str());

	const auto coreClrLib = LoadLibraryExA(libraryPath, nullptr, 0);
	if (!coreClrLib) {
		*err = -1;
		fprintf_s(stderr, "[gmsv_sharp] Unable to locate " CORE_CLR_LIBRARY "\n");
		return false;
	}

	const auto coreClrInit = reinterpret_cast<coreclr_initialize_ptr>(GetProcAddress(coreClrLib, "coreclr_initialize"));
	if (!coreClrInit) {
		*err = -1;
		fprintf_s(stderr, "[gmsv_sharp] Locate coreclr_initialize() failed\n");
		return false;
	}

	pCreateDelegatePtr = reinterpret_cast<coreclr_create_delegate_ptr>(GetProcAddress(coreClrLib, "coreclr_create_delegate"));
	if (!pCreateDelegatePtr) {
		*err = -1;
		fprintf_s(stderr, "[gmsv_sharp] Locate coreclr_create_delegate() failed\n");
		return false;
	}

	pShutdownFn = GetProcAddress(coreClrLib, "coreclr_shutdown");
	const auto coreClrShutdown = reinterpret_cast<coreclr_shutdown_ptr>(pShutdownFn);
	if (!coreClrShutdown) {
		*err = -1;
		fprintf_s(stderr, "[gmsv_sharp] Locate coreclr_shutdown() failed\n");
		return false;
	}

	char runtimeAssemblyPath[MAX_PATH];
	GetModuleFileNameA(nullptr, runtimeAssemblyPath, MAX_PATH);

	// Construct the managed library path
	auto managedLibraryPath(sLibsPath);
	managedLibraryPath.append(FS_SEPARATOR);
	managedLibraryPath.append(sMainLibName);

	// Construct trusted libs path
	std::string tpaList;
	BuildTpaList(sLibsPath.c_str(), ".dll", tpaList);
	BuildTpaList(sSharedLibsPath.c_str(), ".dll", tpaList);
	BuildTpaList(sRootRuntimePath.c_str(), ".dll", tpaList);
	BuildTpaList(sRootRuntimePath.c_str(), ".exe", tpaList);

	const char* propertyKeys[] = {
		"TRUSTED_PLATFORM_ASSEMBLIES"
	};
	const char* propertyValues[] = {
		tpaList.c_str()
	};

	// printf("tpaList.c_str(): %s\n", tpaList.c_str());

	HRESULT result = coreClrInit(runtimeAssemblyPath, "host", sizeof(propertyKeys) / sizeof(char*), propertyKeys, propertyValues, &pHostHandle, &uiDomainId);
	if (result < 0) {
		*err = result;
		fprintf_s(stderr, "[gmsv_sharp] Failed to create CoreCLR runtime!\n");
		return false;
	}

	ModuleInitializePtr moduleInit;
	if (!GetDelegate("GmodSharpCore.Core", "Initialize", reinterpret_cast<void**>(&moduleInit), err)) {
		return false;
	}

	pCorePtr = moduleInit(sModulesLookupPath.c_str(), sSharedLibsPath.c_str(), sLibsPath.c_str(), sMainLibName.c_str());
	return pCorePtr != nullptr;
}

bool gm_sharp::Runtime::GetDelegate(const char* typeName, const char* function, void** fnRef, long* result) const {
	const auto hResult = pCreateDelegatePtr(
		pHostHandle,
		uiDomainId,
		"gmod_sharp_core",
		typeName,
		function,
		fnRef);

	if (hResult < 0) {
		const auto msg = std::system_category().message(hResult);
		if (hResult == __HRESULT_FROM_WIN32(ERROR_CAN_NOT_COMPLETE)) {
			*result = hResult;
			fprintf_s(stderr, "[gmsv_sharp] %s() failed [ERROR_CAN_NOT_COMPLETE: 0x%x]!\n", function, hResult);
			return false;
		}

		if (hResult == __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			*result = hResult;
			fprintf_s(stderr, "[gmsv_sharp] %s() failed [ERROR_FILE_NOT_FOUND: 0x%x]!\n", function, hResult);
			return false;
		}

		const auto winErr = Win32FromHResult(hResult);
		*result = winErr;
		fprintf_s(stderr, "[gmsv_sharp] %s() failed [hresult: 0x%x, win_err: %d] -> %s\n", function, hResult, winErr, msg.c_str());
		return false;
	}

	return true;
}

void gm_sharp::Runtime::Shutdown() const {
	if (!IsValidPtr(pShutdownFn)) {
		return;
	}

	const auto coreClrShutdown = reinterpret_cast<coreclr_shutdown_ptr>(pShutdownFn);
	coreClrShutdown(pHostHandle, uiDomainId);
}

int gm_sharp::Runtime::CompileInline(std::string sCode) const {
	ModuleCompileInlinePtr fn;
	long err;
	if (!GetDelegate("GmodSharpCore.Core", "CompileInline", reinterpret_cast<void**>(&fn), &err)) {
		return -10;
	}

	return fn(sCode.c_str());
}

void gm_sharp::Runtime::BuildTpaList(const char* directory, const char* extension, std::string& tpaList) {
	// This will add all files with a .dll extension to the TPA list.
	// This will include unmanaged assemblies (coreclr.dll, for example) that don't
	// belong on the TPA list. In a real host, only managed assemblies that the host
	// expects to load should be included. Having extra unmanaged assemblies doesn't
	// cause anything to fail, though, so this function just enumerates all dll's in
	// order to keep this sample concise.
	std::string searchPath(directory);
	searchPath.append(FS_SEPARATOR);
	searchPath.append("*");
	searchPath.append(extension);

	WIN32_FIND_DATAA findData;
	const auto fileHandle = FindFirstFileA(searchPath.c_str(), &findData);

	if (fileHandle != INVALID_HANDLE_VALUE) {
		do {
			// Append the assembly to the list
			tpaList.append(directory);
			tpaList.append(FS_SEPARATOR);
			tpaList.append(findData.cFileName);
			tpaList.append(PATH_DELIMITER);

			// Note that the CLR does not guarantee which assembly will be loaded if an assembly
			// is in the TPA list multiple times (perhaps from different paths or perhaps with different NI/NI.dll
			// extensions. Therefore, a real host should probably add items to the list in priority order and only
			// add a file if it's not already present on the list.
			//
			// For this simple sample, though, and because we're only loading TPA assemblies from a single path,
			// and have no native images, we can ignore that complication.
		} while (FindNextFileA(fileHandle, &findData));
		FindClose(fileHandle);
	}
}

DWORD Win32FromHResult(HRESULT hr) {
	if ((hr & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0)) {
		return HRESULT_CODE(hr);
	}

	if (hr == S_OK) {
		return ERROR_SUCCESS;
	}

	// Not a Win32 HRESULT so return a generic error code.
	return ERROR_CAN_NOT_COMPLETE;
}
