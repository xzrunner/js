#include "js/RapidJsonHelper.h"

#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

#include <cstdio>

namespace js
{

// todo: for multithread
static char BUFFER[65536];

void RapidJsonHelper::ReadFromFile(const char* filepath, rapidjson::Document& doc)
{
	FILE* fp = fopen(filepath, "rb");
	rapidjson::FileReadStream read(fp, BUFFER, sizeof(BUFFER));

	doc.ParseStream(read);

	fclose(fp);
}

void RapidJsonHelper::WriteToFile(const char* filepath, const rapidjson::Document& doc, bool pretty)
{
	FILE* fp = fopen(filepath, "wb");
	rapidjson::FileWriteStream fout(fp, BUFFER, sizeof(BUFFER));

	if (pretty) {
		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(fout);
		doc.Accept(writer);
	} else {
		rapidjson::Writer<rapidjson::FileWriteStream> writer(fout);
		doc.Accept(writer);
	}

	fclose(fp);
}

}