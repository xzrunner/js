#include "js/RTTR.h"

namespace detail
{

extern std::string rttr_to_rapidjson(rttr::instance obj, const std::string& dir_path);
extern bool rttr_from_rapidjson(const std::string& json, const std::string& dir_path, rttr::instance obj);

}

namespace js
{

const char* RTTR::FILEPATH_TAG     = "IS_FILEPATH";
const char* RTTR::NO_SERIALIZE_TAG = "NO_SERIALIZE";

std::string RTTR::ToRapidJson(rttr::instance obj, const std::string& dir_path)
{
	return detail::rttr_to_rapidjson(obj, dir_path);
}

bool RTTR::FromRapidJson(const std::string& json, const std::string& dir_path, rttr::instance obj)
{
	return detail::rttr_from_rapidjson(json, dir_path, obj);
}

}