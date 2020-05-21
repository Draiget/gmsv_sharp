#ifndef ZONTWELG_GM_SHARP_RUNTIME_H
#define ZONTWELG_GM_SHARP_RUNTIME_H

#include <string>
#include <coreclrhost.h>

namespace gm_sharp
{
	class Runtime
	{
	public:
		Runtime(std::string sRootPath, std::string sSharedLibsPath, std::string sLibsPath, std::string sMainLibName, std::string sModulesLookupPath);

		bool Initialize(long* err);
		void Shutdown() const;

		int CompileInline(std::string sCode) const;

	private:
		std::string sRootRuntimePath;
		std::string sSharedLibsPath;
		std::string sLibsPath;
		std::string sMainLibName;
		std::string sModulesLookupPath;

		coreclr_create_delegate_ptr pCreateDelegatePtr;
		void* pCorePtr;
		void* pShutdownFn;
		void* pHostHandle;
		unsigned int uiDomainId;

		static void BuildTpaList(const char* directory, const char* extension, std::string& tpaList);
		bool GetDelegate(const char* typeName, const char* function, void** fnRef, long* result) const;
	};
}

#endif
