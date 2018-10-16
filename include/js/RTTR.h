#pragma once

#include <rttr/type>

namespace js
{

class RTTR
{
public:
	static std::string ToRapidJson(rttr::instance obj);
	static bool FromRapidJson(const std::string& json, const std::string& dir_path, rttr::instance obj);

	static const char* FILEPATH_TAG;

}; // RTTR

}