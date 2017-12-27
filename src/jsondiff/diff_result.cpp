#include <jsondiff/diff_result.h>
#include <jsondiff/json_value_types.h>
#include <jsondiff/helper.h>
#include <sstream>

namespace jsondiff
{
	DiffResult::DiffResult()
		: _is_undefined(true)
	{
	}

	DiffResult::DiffResult(JsonValue diff_json) :
		_diff_json(diff_json)
	{
		if (diff_json.is_null())
			_is_undefined = true;
		else
			_is_undefined = false;
	}

	std::shared_ptr<DiffResult> DiffResult::make_undefined_diff_result()
	{
		auto result = std::make_shared<DiffResult>();
		result->_is_undefined = true;
		return result;
	}

	std::string DiffResult::str() const
	{
		return fc::json::to_string(_diff_json);
	}

	std::string DiffResult::pretty_str() const
	{
		return fc::json::to_pretty_string(_diff_json);
	}

	bool DiffResult::is_undefined() const
	{
		return _is_undefined;
	}

	JsonValue DiffResult::value() const
	{
		return _diff_json;
	}

	std::string DiffResult::pretty_diff_str(size_t indent_count) const
	{
		// TODO: indents
		std::stringstream indent_ss;
		for (size_t i = 0; i < indent_count; i++)
			indent_ss << "\t";
		auto indents = indent_ss.str();
		auto diff_json_type = guess_json_value_type(_diff_json);
		std::stringstream ss;
		if (is_scalar_json_value_type(diff_json_type))
		{
			ss << indents  << " " << json_dumps(_diff_json);
			return ss.str();
		}
		else if (diff_json_type == JsonValueType::JVT_OBJECT)
		{
			auto diff_json_obj = _diff_json.as<fc::mutable_variant_object>();
			if (is_scalar_value_diff_format(_diff_json))
			{
				ss << indents << "-" << json_dumps(diff_json_obj[JSONDIFF_KEY_OLD_VALUE]) << std::endl;
				ss << indents << "+" << json_dumps(diff_json_obj[JSONDIFF_KEY_NEW_VALUE]) << std::endl;
				return ss.str();
			}
			for (auto i = diff_json_obj.begin(); i != diff_json_obj.end(); i++)
			{
				auto key = i->key();
				auto diff_item = i->value();
				if (utils::string_ends_with(key, JSONDIFF_KEY_ADDED_POSTFIX) && key.size() > strlen(JSONDIFF_KEY_ADDED_POSTFIX))
				{
					auto origin_key = utils::string_without_ext(key, JSONDIFF_KEY_ADDED_POSTFIX);
					ss << indents << "\t+" << origin_key << ":" << json_dumps(diff_item) << std::endl;
					continue;
				}
				else if (utils::string_ends_with(key, JSONDIFF_KEY_DELETED_POSTFIX) && key.size() > strlen(JSONDIFF_KEY_DELETED_POSTFIX))
				{
					auto origin_key = utils::string_without_ext(key, JSONDIFF_KEY_DELETED_POSTFIX);
					ss << indents << "\t-" << origin_key << ":" << json_dumps(diff_item) << std::endl;
					continue;
				}
				ss << indents << "\t" << key << ":" << std::endl << std::make_shared<DiffResult>(diff_item)->pretty_diff_str(indent_count+1) << std::endl;
			}
			return ss.str();
		}
		else if (diff_json_type == JsonValueType::JVT_ARRAY)
		{
			auto diff_json_array = _diff_json.as<fc::variants>();
			for (size_t i = 0; i < diff_json_array.size(); i++)
			{
				if (!diff_json_array[i].is_array())
				{
					ss << indents << "\t " << json_dumps(diff_json_array[i]) << std::endl;
					continue;
				}
				auto diff_item = diff_json_array[i].as<fc::variants>();
				if (diff_item.size() != 3)
				{
					ss << indents << "\t " << json_dumps(diff_json_array[i]) << std::endl;
					continue;
				}
				auto op_item = diff_item[0].as_string();
				auto pos = diff_item[1].as_uint64();
				auto inner_diff_json = diff_item[2];
				if (op_item == std::string("+"))
				{
					ss << indents << "\t+" << json_dumps(inner_diff_json) << std::endl;
				}
				else if (op_item == std::string("-"))
				{
					ss << indents << "\t-" << json_dumps(inner_diff_json) << std::endl;
				}
				else if (op_item == std::string("~"))
				{
					ss << indents << "\t~" << std::endl << std::make_shared<DiffResult>(inner_diff_json)->pretty_diff_str(indent_count+1) << std::endl;
				}
				else
				{
					ss << indents << "\t " << json_dumps(diff_json_array[i]) << std::endl;
					continue;
				}
			}
			return ss.str();
		}
		else
		{
			ss << indents << json_dumps(_diff_json);
			return ss.str();
		}
	}

	DiffResult::~DiffResult()
	{

	}
}
