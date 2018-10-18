#pragma once

#include <rttr/type>

namespace js
{

class RTTR
{
public:
	static std::string ToRapidJson(rttr::instance obj, const std::string& dir_path = "");
	static bool FromRapidJson(const std::string& json, const std::string& dir_path, rttr::instance obj);

	static const char* FilePathTag() {
		return "IS_FILEPATH";
	}
	static const char* NoSerializeTag() {
		return "NO_SERIALIZE";
	}

}; // RTTR

}