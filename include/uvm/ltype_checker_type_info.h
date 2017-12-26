#ifndef ltype_checker_type_info_h
#define ltype_checker_type_info_h

#include <uvm/lprefix.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <list>
#include <vector>
#include <unordered_set>
#include <queue>
#include <memory>
#include <functional>
#include <algorithm>

#include <uvm/llimits.h>
#include <uvm/lstate.h>		  
#include <uvm/lua.h>
#include <uvm/uvm_api.h>
#include <uvm/uvm_tokenparser.h>
#include <uvm/lparsercombinator.h>
#include <uvm/exceptions.h>
#include <uvm/uvm_debug_file.h>
#include <uvm/uvm_lutil.h>

namespace uvm {
	namespace parser {

		typedef ::GluaTypeInfoEnum GluaTypeInfoEnum;

		struct GluaTypeInfo;


		typedef GluaTypeInfo* GluaTypeInfoP;

		struct GluaTypeInfo
		{
			GluaTypeInfoEnum etype;

			// function type fields
			std::vector<std::string> arg_names;
			// ѭãǶ׺objectͣGluaTypeCheckerм¼ϢĶȻʹGluaTypeInfo *
			std::vector<GluaTypeInfoP> arg_types;
			std::vector<GluaTypeInfoP> ret_types;
			bool is_offline;
			bool declared;
			bool is_any_function;
			bool is_any_contract; // ΪԼʹãtable
			bool is_stream_type; // ǷǶͣҲ֮һ

			bool is_literal_token_value; // Ƿǵtokenֵ
			GluaParserToken literal_value_token; // ǵtokenʽֵtoken

			// end function type fields

			// record͵ĸ
			std::unordered_map<std::string, GluaTypeInfoP> record_props;
			std::unordered_map<std::string, std::string> record_default_values; // recordԵĬֵ
			std::string record_name; // recordƣtypedef
			std::string record_origin_name; // recordԭʼƣõͣǷȫչƣG1<G2<Person, string>, string, int>
			std::vector<GluaTypeInfoP> record_generics; // recordõķ
			std::vector<GluaTypeInfoP> record_all_generics; // recordʹʱзͲ
			std::vector<GluaTypeInfoP> record_applied_generics; // recordʹʵзͲ

			// ͵
			std::string generic_name; // 

			// б͵
			GluaTypeInfoP array_item_type; // беÿһ

			// Map͵
			GluaTypeInfoP map_item_type; // Mapеֵ
			bool is_literal_empty_table; // ǷյArray/Map

			// literal type
			std::vector<GluaParserToken> literal_type_options; // literal typeĿѡֵ

			std::unordered_set<GluaTypeInfoP> union_types; // may be any one of these types, not supported nested full info function now

			GluaTypeInfo(GluaTypeInfoEnum type_info_enum = GluaTypeInfoEnum::LTI_OBJECT);

			bool is_contract_type() const;

			bool is_function() const;

			bool is_int() const;

			bool is_number() const;

			bool is_bool() const;

			bool is_string() const;

			// literal typeеĿ԰ÿһ
			bool is_literal_item_type() const;

			// TODO: __call__ͷԪʹڲʱҪغж

			bool has_call_prop() const;

			bool may_be_callable() const;

			bool is_nil() const;

			bool is_undefined() const;

			bool is_union() const;

			bool is_record() const;

			bool is_array() const;

			bool is_map() const;
				
			bool is_table() const;

			// tableͣetypeLTI_TABLE
			bool is_narrow_table() const;

			// tableͣtable, record, Map, Array
			bool is_like_table() const;
				
			bool is_generic() const;

			bool is_literal_type() const;

			// жϺǷ...Ƿ)
			bool has_var_args() const;

			// Ԫ(__ͷĳԱԶrecord͵metatable)
			bool has_meta_method() const;

			bool is_same_record(GluaTypeInfoP other) const;

			size_t min_args_count_require() const;

			// literal typeǷƥֵ
			bool match_literal_type(GluaTypeInfoP value_type) const;
			// literal typeֵǵtokenǷƥ
			bool match_literal_value(GluaParserToken value_token) const;

			// literal typeǷĳvalue_type
			bool contains_literal_item_type(GluaTypeInfoP value_type) const;

			// @param show_record_details Ƿʾrecord͵ϸΪ˱ݹѭ
			std::string str(const bool show_record_details = false) const;

			// ѺԼstorageϢuvmģ
			// @throws LuaException
			bool put_contract_storage_type_to_module_stream(GluaModuleByteStreamP stream);

			// ѺԼstorageAPIsϢuvmģ
			// @throws LuaException
			bool put_contract_apis_info_to_module_stream(GluaModuleByteStreamP stream);

		};

	} // end namespace uvm::parser
}

#endif
