#pragma once

namespace Json { class Value; }

namespace js
{

class JsonCppHelper
{
public:
	static void ReadFromFile(const char* filepath, Json::Value& val);

	static void WriteToFile(const char* filepath, const Json::Value& val);

}; // JsonCppHelper

}