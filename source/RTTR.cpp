#include "js/RTTR.h"

namespace detail
{

extern std::string rttr_to_rapidjson(rttr::instance obj);
extern bool rttr_from_rapidjson(const std::string& json, rttr::instance obj);

}

namespace js
{

std::string RTTR::ToRapidJson(rttr::instance obj)
{
	return detail::rttr_to_rapidjson(obj);
}

bool RTTR::FromRapidJson(const std::string& json, rttr::instance obj)
{
	return detail::rttr_from_rapidjson(json, obj);
}

}