#pragma once

#include <rttr/type>

namespace js
{

class RTTR
{
public:
	static std::string ToRapidJson(rttr::instance obj);
	static bool FromRapidJson(const std::string& json, rttr::instance obj);

}; // RTTR

}