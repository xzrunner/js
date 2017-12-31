#include "js/JsonCppHelper.h"

#include <json/reader.h>
#include <json/writer.h>

#include <fstream>

namespace js
{

void JsonCppHelper::ReadFromFile(const char* filepath, Json::Value& val)
{
	Json::Value value;
	Json::Reader reader;
	std::locale::global(std::locale(""));
	std::ifstream fin(filepath);
	std::locale::global(std::locale("C"));
	reader.parse(fin, val);
	fin.close();
}

void JsonCppHelper::WriteToFile(const char* filepath, const Json::Value& val)
{
	Json::StyledStreamWriter writer;
	std::locale::global(std::locale(""));
	std::ofstream fout(filepath);
	std::locale::global(std::locale("C"));
	writer.write(fout, val);
	fout.close();
}

}