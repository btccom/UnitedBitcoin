#ifndef JSONDIFF_JSONDIFF_H
#define JSONDIFF_JSONDIFF_H

#include <jsondiff/config.h>
#include <jsondiff/diff_result.h>
#include <jsondiff/json_value_types.h>

#include <string>
#include <memory>

#include <fc/io/json.hpp>
#include <fc/string.hpp>
#include <fc/variant.hpp>
#include <fc/variant_object.hpp>


namespace jsondiff
{
	class JsonDiff
	{
	private:

	public:
		JsonDiff();
		virtual ~JsonDiff();


		DiffResultP diff_by_string(const std::string &old_json_str, const std::string &new_json_str);

		// @throws JsonDiffException
		DiffResultP diff(JsonValue old_json, JsonValue new_json);

		JsonValue patch_by_string(std::string old_json_value, DiffResultP diff_info);

		// @throws JsonDiffException
		JsonValue patch(JsonValue old_json, DiffResultP diff_info);

		JsonValue rollback_by_string(std::string new_json_value, DiffResultP diff_info);

		// @throws JsonDiffException
		JsonValue rollback(JsonValue new_json, DiffResultP diff_info);

	};
}

#endif
