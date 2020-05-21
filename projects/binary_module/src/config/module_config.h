#ifndef ZONTWELG_SHARP_MODULE_CONFIG_H
#define ZONTWELG_SHARP_MODULE_CONFIG_H

#include <string>

/*
 * Static configuration
 */
#define GM_SHARP_SERVER_CONFIG_PATH "sharp_config.json"

/*
 * Forwards
 */
namespace Json
{
	class Value;
}

/*
 * Module configuration that contains CoreCLR\Host paths and etc.
 */
namespace gm_sharp
{
	class ModuleConfig
	{
	public:
		static bool Initialize();
		static bool Reload();

		static std::string GetClrRootPath();
		static std::string GetClrLibsPath();
		static std::string GetApiLibsPath();
		static std::string GetApiCoreLibName();
		static std::string GetApiModulesLookupPath();

	private:
		static std::string sCoreClrPathRoot;
		static std::string sCoreClrPathLibs;
		static std::string sApiLibsPath;
		static std::string sApiMainLibName;
		static std::string sModulesLookupPath;

		static Json::Value* oRootConfigNode;
	};
}

#endif
