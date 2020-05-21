#include "module_config.h"
#include "../utils/utils_boost_fs.h"

#include "../common/json/include/json/value.h"
#include "../common/json/include/json/reader.h"
#include "../common/json/include/json/json.h"
#include "../utils/utils_validating.h"

Json::Value* gm_sharp::ModuleConfig::oRootConfigNode = nullptr;
std::string gm_sharp::ModuleConfig::sCoreClrPathRoot;
std::string gm_sharp::ModuleConfig::sCoreClrPathLibs;
std::string gm_sharp::ModuleConfig::sApiLibsPath;
std::string gm_sharp::ModuleConfig::sApiMainLibName;
std::string gm_sharp::ModuleConfig::sModulesLookupPath;

bool gm_sharp::ModuleConfig::Initialize() {
	auto filePath = fs::path(GM_SHARP_SERVER_CONFIG_PATH);

	// Validate file existence
	if (!fs::exists(GM_SHARP_SERVER_CONFIG_PATH)) {
		printf("[gmsv_sharp] Config file '" GM_SHARP_SERVER_CONFIG_PATH "' is not exists, please, create it manually\n");
		return false;
	}

	return Reload();
}

bool gm_sharp::ModuleConfig::Reload() {
	std::ifstream configDoc(GM_SHARP_SERVER_CONFIG_PATH, std::ifstream::binary);

	oRootConfigNode = new Json::Value();
	std::string parseErrors;
	const Json::CharReaderBuilder builder;
	try {
		if (!Json::parseFromStream(builder, configDoc, oRootConfigNode, &parseErrors)) {
			printf("[GlobalAdmin] Unable to parse config file '" GM_SHARP_SERVER_CONFIG_PATH "', json error: %s\n", parseErrors.c_str());
			return false;
		}

		sCoreClrPathRoot = (*oRootConfigNode)["clr_runtime_path"].asCString();
		sCoreClrPathLibs = (*oRootConfigNode)["clr_runtime_shared_libs_path"].asCString();
		sApiLibsPath = (*oRootConfigNode)["sharp_libs_path"].asCString();
		sApiMainLibName = (*oRootConfigNode)["sharp_main_lib"].asCString();
		sModulesLookupPath = (*oRootConfigNode)["sharp_modules_lookup_path"].asCString();
	}
	catch (std::exception & err) {
		printf("[GlobalAdmin] Unable to parse config file '" GM_SHARP_SERVER_CONFIG_PATH "', exception: %s\n", err.what());
		return false;
	}

	return true;
}

std::string gm_sharp::ModuleConfig::GetClrRootPath() {
	if (!IsValidPtr(oRootConfigNode)) {
		return "";
	}

	return sCoreClrPathRoot;
}

std::string gm_sharp::ModuleConfig::GetClrLibsPath() {
	if (!IsValidPtr(oRootConfigNode)) {
		return "";
	}

	return sCoreClrPathLibs;
}

std::string gm_sharp::ModuleConfig::GetApiLibsPath() {
	if (!IsValidPtr(oRootConfigNode)) {
		return "";
	}

	return sApiLibsPath;
}

std::string gm_sharp::ModuleConfig::GetApiCoreLibName() {
	if (!IsValidPtr(oRootConfigNode)) {
		return "";
	}

	return sApiMainLibName;
}

std::string gm_sharp::ModuleConfig::GetApiModulesLookupPath() {
	if (!IsValidPtr(oRootConfigNode)) {
		return "";
	}

	return sModulesLookupPath;
}
