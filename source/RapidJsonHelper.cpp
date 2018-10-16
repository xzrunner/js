#include "js/RapidJsonHelper.h"

#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

#include <cstdio>

namespace js
{

// todo: for multithread
static char BUFFER[65536];

bool RapidJsonHelper::ReadFromFile(const char* filepath, rapidjson::Document& doc)
{
	FILE* fp = fopen(filepath, "rb");
	if (!fp) {
		return false;
	}
	rapidjson::FileReadStream read(fp, BUFFER, sizeof(BUFFER));

	doc.ParseStream(read);

	fclose(fp);

	return true;
}

bool RapidJsonHelper::WriteToFile(const char* filepath, const rapidjson::Document& doc, bool pretty)
{
	FILE* fp = fopen(filepath, "wb");
	if (!fp) {
		return false;
	}
	rapidjson::FileWriteStream fout(fp, BUFFER, sizeof(BUFFER));

	if (pretty) {
		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(fout);
		doc.Accept(writer);
	} else {
		rapidjson::Writer<rapidjson::FileWriteStream> writer(fout);
		doc.Accept(writer);
	}

	fclose(fp);

	return true;
}

std::string RapidJsonHelper::ValueToString(const rapidjson::Value& val)
{
	rapidjson::StringBuffer buf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
	val.Accept(writer);
	return buf.GetString();
}

}