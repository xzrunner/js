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

void fromjson_recursively(instance obj, Value& json_object, const std::string& dir_path);

/////////////////////////////////////////////////////////////////////////////////////////

variant extract_basic_types(Value& json_value)
{
    switch(json_value.GetType())
    {
        case kStringType:
        {
            return std::string(json_value.GetString());
            break;
        }
        case kNullType:     break;
        case kFalseType:
        case kTrueType:
        {
            return json_value.GetBool();
            break;
        }
        case kNumberType:
        {
            if (json_value.IsInt())
                return json_value.GetInt();
            else if (json_value.IsDouble())
                return json_value.GetDouble();
            else if (json_value.IsUint())
                return json_value.GetUint();
            else if (json_value.IsInt64())
                return json_value.GetInt64();
            else if (json_value.IsUint64())
                return json_value.GetUint64();
            break;
        }
        // we handle only the basic types here
        case kObjectType:
        case kArrayType: return variant();
    }

    return variant();
}


/////////////////////////////////////////////////////////////////////////////////////////

static void write_array_recursively(variant_sequential_view& view, Value& json_array_value, const std::string& dir_path)
{
    view.set_size(json_array_value.Size());
    const type array_value_type = view.get_rank_type(1);

    for (SizeType i = 0; i < json_array_value.Size(); ++i)
    {
        auto& json_index_value = json_array_value[i];
        if (json_index_value.IsArray())
        {
            auto sub_array_view = view.get_value(i).create_sequential_view();
            write_array_recursively(sub_array_view, json_index_value, dir_path);
        }
        else if (json_index_value.IsObject())
        {
            variant var_tmp = view.get_value(i);
            variant wrapped_var = var_tmp.extract_wrapped_value();
            fromjson_recursively(wrapped_var, json_index_value, dir_path);
            view.set_value(i, wrapped_var);
        }
        else
        {
            variant extracted_value = extract_basic_types(json_index_value);
            if (extracted_value.convert(array_value_type))
                view.set_value(i, extracted_value);
        }
    }
}

variant extract_value(Value::MemberIterator& itr, const type& t, const std::string& dir_path)
{
    auto& json_value = itr->value;
    variant extracted_value = extract_basic_types(json_value);
    const bool could_convert = extracted_value.convert(t);
    if (!could_convert)
    {
        if (json_value.IsObject())
        {
            constructor ctor = t.get_constructor();
            for (auto& item : t.get_constructors())
            {
                if (item.get_instantiated_type() == t)
                    ctor = item;
            }
            extracted_value = ctor.invoke();
            fromjson_recursively(extracted_value, json_value, dir_path);
        }
    }

    return extracted_value;
}

static void write_associative_view_recursively(variant_associative_view& view, Value& json_array_value, const std::string& dir_path)
{
    for (SizeType i = 0; i < json_array_value.Size(); ++i)
    {
        auto& json_index_value = json_array_value[i];
        if (json_index_value.IsObject()) // a key-value associative view
        {
            Value::MemberIterator key_itr = json_index_value.FindMember("key");
            Value::MemberIterator value_itr = json_index_value.FindMember("value");

            if (key_itr != json_index_value.MemberEnd() &&
                value_itr != json_index_value.MemberEnd())
            {
                auto key_var = extract_value(key_itr, view.get_key_type(), dir_path);
                auto value_var = extract_value(value_itr, view.get_value_type(), dir_path);
                if (key_var && value_var)
                {
                    view.insert(key_var, value_var);
                }
            }
        }
        else // a key-only associative view
        {
            variant extracted_value = extract_basic_types(json_index_value);
            if (extracted_value && extracted_value.convert(view.get_key_type()))
                view.insert(extracted_value);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

void fromjson_recursively(instance obj2, Value& json_object, const std::string& dir_path)
{
    instance obj = obj2.get_type().get_raw_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;
    const auto prop_list = obj.get_derived_type().get_properties();

    for (auto prop : prop_list)
    {
        Value::MemberIterator ret = json_object.FindMember(prop.get_name().data());
        if (ret == json_object.MemberEnd())
            continue;
        const type value_t = prop.get_type();
		auto& json_value = ret->value;

		if (prop.get_metadata(js::RTTR::FilePathTag()))
		{
			assert(json_value.GetType() == kStringType);
			auto relative_path = std::string(json_value.GetString());
			auto absolute_path = boost::filesystem::absolute(relative_path, dir_path).string();
			variant extracted_value = absolute_path.c_str();
			if (extracted_value.convert(value_t)) // REMARK: CONVERSION WORKS ONLY WITH "const type", check whether this is correct or not!
				prop.set_value(obj, extracted_value);

			continue;
		}

        switch(json_value.GetType())
        {
            case kArrayType:
            {
                variant var;
                if (value_t.is_sequential_container())
                {
                    var = prop.get_value(obj);
                    auto view = var.create_sequential_view();
                    write_array_recursively(view, json_value, dir_path);
                }
                else if (value_t.is_associative_container())
                {
                    var = prop.get_value(obj);
                    auto associative_view = var.create_associative_view();
                    write_associative_view_recursively(associative_view, json_value, dir_path);
                }

                prop.set_value(obj, var);
                break;
            }
            case kObjectType:
            {
                variant var = prop.get_value(obj);
                fromjson_recursively(var, json_value, dir_path);
                prop.set_value(obj, var);
                break;
            }
            default:
            {
                variant extracted_value = extract_basic_types(json_value);
                if (extracted_value.convert(value_t)) // REMARK: CONVERSION WORKS ONLY WITH "const type", check whether this is correct or not!
                    prop.set_value(obj, extracted_value);
            }
        }
    }
}

} // end namespace anonymous

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

bool rttr_from_rapidjson(const std::string& json, const std::string& dir_path, rttr::instance obj)
{
    Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.

    // "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream().
    if (document.Parse(json.c_str()).HasParseError())
        return 1;

    fromjson_recursively(obj, document, dir_path);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

}
