#pragma once

#include <rttr/type>

namespace js
{

class RTTR
{
public:
	static std::string ToRapidJson(rttr::instance obj, const std::string& dir_path = "");
	static bool FromRapidJson(const std::string& json, const std::string& dir_path, rttr::instance obj);

	static const char* FILEPATH_TAG;
	static const char* NO_SERIALIZE_TAG;

}; // RTTR

}