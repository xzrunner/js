// use code from rttr's sample
// https://github.com/rttrorg/rttr

#include "js/RTTR.h"

#include <cstdio>
#include <string>
#include <vector>
#include <array>

#include <iostream>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h> // for stringify JSON
#include <rapidjson/document.h>     // rapidjson's DOM-style API
#include <rttr/type>

#include <boost/filesystem.hpp>

using namespace rapidjson;
using namespace rttr;

namespace
{

/////////////////////////////////////////////////////////////////////////////////////////

void to_json_recursively(const instance& obj, PrettyWriter<StringBuffer>& writer, const std::string& dir_path);

/////////////////////////////////////////////////////////////////////////////////////////

bool write_variant(const variant& var, PrettyWriter<StringBuffer>& writer, const std::string& dir_path);

bool write_atomic_types_to_json(const type& t, const variant& var, PrettyWriter<StringBuffer>& writer)
{
    if (t.is_arithmetic())
    {
        if (t == type::get<bool>())
            writer.Bool(var.to_bool());
        else if (t == type::get<char>())
            writer.Bool(var.to_bool());
        else if (t == type::get<int8_t>())
            writer.Int(var.to_int8());
        else if (t == type::get<int16_t>())
            writer.Int(var.to_int16());
        else if (t == type::get<int32_t>())
            writer.Int(var.to_int32());
        else if (t == type::get<int64_t>())
            writer.Int64(var.to_int64());
        else if (t == type::get<uint8_t>())
            writer.Uint(var.to_uint8());
        else if (t == type::get<uint16_t>())
            writer.Uint(var.to_uint16());
        else if (t == type::get<uint32_t>())
            writer.Uint(var.to_uint32());
        else if (t == type::get<uint64_t>())
            writer.Uint64(var.to_uint64());
        else if (t == type::get<float>())
            writer.Double(var.to_double());
        else if (t == type::get<double>())
            writer.Double(var.to_double());

        return true;
    }
    else if (t.is_enumeration())
    {
        bool ok = false;
        auto result = var.to_string(&ok);
        if (ok)
        {
            writer.String(var.to_string());
        }
        else
        {
            ok = false;
            auto value = var.to_uint64(&ok);
            if (ok)
                writer.Uint64(value);
            else
                writer.Null();
        }

        return true;
    }
    else if (t == type::get<std::string>())
    {
        writer.String(var.to_string());
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void write_array(const variant_sequential_view& view, PrettyWriter<StringBuffer>& writer, const std::string& dir_path)
{
    writer.StartArray();
    for (const auto& item : view)
    {
        if (item.is_sequential_container())
        {
            write_array(item.create_sequential_view(), writer, dir_path);
        }
        else
        {
            variant wrapped_var = item.extract_wrapped_value();
            type value_type = wrapped_var.get_type();
            if (value_type.is_arithmetic() || value_type == type::get<std::string>() || value_type.is_enumeration())
            {
                write_atomic_types_to_json(value_type, wrapped_var, writer);
            }
            else // object
            {
                to_json_recursively(wrapped_var, writer, dir_path);
            }
        }
    }
    writer.EndArray();
}


/////////////////////////////////////////////////////////////////////////////////////////

static void write_associative_container(const variant_associative_view& view, PrettyWriter<StringBuffer>& writer, const std::string& dir_path)
{
    static const string_view key_name("key");
    static const string_view value_name("value");

    writer.StartArray();

    if (view.is_key_only_type())
    {
        for (auto& item : view)
        {
            write_variant(item.first, writer, dir_path);
        }
    }
    else
    {
        for (auto& item : view)
        {
            writer.StartObject();
            writer.String(key_name.data(), static_cast<rapidjson::SizeType>(key_name.length()), false);

            write_variant(item.first, writer, dir_path);

            writer.String(value_name.data(), static_cast<rapidjson::SizeType>(value_name.length()), false);

            write_variant(item.second, writer, dir_path);

            writer.EndObject();
        }
    }

    writer.EndArray();
}

/////////////////////////////////////////////////////////////////////////////////////////

bool write_variant(const variant& var, PrettyWriter<StringBuffer>& writer, const std::string& dir_path)
{
    auto value_type = var.get_type();
    auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
    bool is_wrapper = wrapped_type != value_type;
    if (write_atomic_types_to_json(is_wrapper ? wrapped_type : value_type,
                                   is_wrapper ? var.extract_wrapped_value() : var, writer))
    {
    }
    else if (var.is_sequential_container())
    {
        write_array(var.create_sequential_view(), writer, dir_path);
    }
    else if (var.is_associative_container())
    {
        write_associative_container(var.create_associative_view(), writer, dir_path);
    }
    else
    {
        auto child_props = is_wrapper ? wrapped_type.get_properties() : value_type.get_properties();
        if (!child_props.empty())
        {
            to_json_recursively(var, writer, dir_path);
        }
        else
        {
			if (var.is_type<const char*>())
			{
				auto text = var.get_value<const char*>();
				writer.String(text);
			}
			else if (var.is_type<std::string>())
			{
				auto text = var.get_value<std::string>();
				writer.String(text);
			}
			else
			{
				writer.String("");
				return false;
			}
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void to_json_recursively(const instance& obj2, PrettyWriter<StringBuffer>& writer, const std::string& dir_path)
{
    writer.StartObject();
    instance obj = obj2.get_type().get_raw_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;

    auto prop_list = obj.get_derived_type().get_properties();
    for (auto prop : prop_list)
    {
		if (prop.get_metadata(js::RTTR::NoSerializeTag())) {
			continue;
		}

        variant prop_value = prop.get_value(obj);
        if (!prop_value)
            continue; // cannot serialize, because we cannot retrieve the value

        const auto name = prop.get_name();
        writer.String(name.data(), static_cast<rapidjson::SizeType>(name.length()), false);
        auto zz = strncmp(name.data(), js::RTTR::FILEPATH_STR, name.length());
        bool is_filepath = strncmp(name.data(), js::RTTR::FILEPATH_STR, name.length()) == 0;
		if (is_filepath || prop.get_metadata(js::RTTR::FilePathTag()))
		{
            std::string path;
			if (prop_value.is_type<const char*>()) {
				path = prop_value.get_value<const char*>();
			} else if (prop_value.is_type<std::string>()) {
				path = prop_value.get_value<std::string>();
			} else {
				assert(0);
			}
            if (dir_path.empty()) {
                writer.String(path);
            } else {
                auto relative = boost::filesystem::relative(path, dir_path).string();
                if (relative.empty()) {
                    relative = path;
                }
                writer.String(relative);
            }
		}
        else if (!write_variant(prop_value, writer, dir_path))
        {
            std::cerr << "cannot serialize property: " << name << std::endl;
        }
    }

    writer.EndObject();
}

} // end namespace anonymous

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

/////////////////////////////////////////////////////////////////////////////////////////

std::string rttr_to_rapidjson(rttr::instance obj, const std::string& dir_path)
{
    if (!obj.is_valid())
        return std::string();

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);

    to_json_recursively(obj, writer, dir_path);

    return sb.GetString();
}

/////////////////////////////////////////////////////////////////////////////////////////

}
