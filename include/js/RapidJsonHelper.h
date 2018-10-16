#pragma once

#include <rapidjson/document.h>

namespace js
{

class RapidJsonHelper
{
public:
	static bool ReadFromFile(const char* filepath, rapidjson::Document& doc);

	static bool WriteToFile(const char* filepath, const rapidjson::Document& doc, bool pretty = true);

	static std::string ValueToString(const rapidjson::Value& val);

}; // RapidJson

}