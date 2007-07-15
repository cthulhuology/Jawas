// objects.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

JObject object_obj = {
	0,
	{ RED(Get),		PRIM(obj_get) },
	{ RED(Put),		PRIM(obj_put) },
	{ RED(Prototype),	PRIM(obj_prototype) },
	{ RED(Class),		PRIM(obj_class) },
	{ RED(CanPut),		PRIM(obj_can_put) },
	{ RED(HasProperty),	PRIM(obj_has_prop) },
	{ RED(Delete),		PRIM(obj_delete) },
	{ RED(DefaultValue),	PRIM(obj_default_value) },
	{ RED(HasInstance),	PRIM(obj_has_instance) },
	{ RED(Scope),		PRIM(obj_scope) },
	{ RED(Match),		PRIM(obj_match) },
	{ RED(Eval),		FUNC(obj_eval) },
	{ RED(Construct),	FUNC(obj_construct) },
	{ RED(toString),	FUNC(obj_toString) },
	{ RED(toBoolean),	FUNC(obj_toBoolean) },
	{ RED(toInteger),	FUNC(obj_toInteger) },
	{ RED(toNumber),	FUNC(obj_toNumber) },
	{ RED(valueOf),		FUNC(obj_value_of) },
	{ RED(hasOwnProperty),	FUNC(obj_has_own_prop) },
	{ RED(isPrototypeOf),	FUNC(obj_is_proto_of) },
	{ RED(propertyIsEnum),	FUNC(obj_prop_is_enum) },
	{ RED(Value),		Undefined },
	{ RED(Length),		One },
	{ NULL, NULL }
};

JObject string_obj = {
	0,
	{ RED(Construct),	FUNC(str_construct) },
	{ RED(SubString),	FUNC(str_substring) },
	{ RED(Split),		FUNC(str_split) },
	{ RED(Slice),		FUNC(str_slice) },
	{ RED(Search),		FUNC(str_search) },
	{ RED(Replace),		FUNC(str_replace) },
	{ RED(indexOf),		FUNC(str_index_of) },
	{ RED(lastIndexOf),	FUNC(str_last_index_of) },
	{ RED(Concat),		FUNC(str_concat) },
	{ RED(charAt),		FUNC(str_char_at) },
	{ RED(charCodeAt),	FUNC(str_char_code_at) },
	{ RED(encodeURI),	FUNC(str_encodeURI) },
	{ RED(decodeURI),	FUNC(str_decodeURI) },
	{ RED(toLowerCase),	FUNC(str_tolower) },
	{ RED(toUpperCase),	FUNC(str_toupper) },
	{ RED(Length),		Zero },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject boolean_obj = {
	0,
	{ RED(Construct),	FUNC(bool_construct) },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject integer_obj = {
	0,
	{ RED(Construct),	FUNC(int_construct) },
	{ NULL, number_obj },
	{ NULL, NULL }
};

JObject number_obj = {
	0,
	{ RED(Construct),	FUNC(num_construct) },
	{ RED(isNaN),		FUNC(obj_isNan) },
	{ RED(isFinite),	FUNC(obj_isFinite) },
	{ RED(toFixed),		FUNC(num_to_fixed) },
	{ RED(toExponential),	FUNC(num_to_exp) },
	{ RED(toPrecision),	FUNC(num_to_precision) },
	{ RED(NaN),		NaN,
	{ RED(Infinity),	Infinity,
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject function_obj = {
	0,
	{ RED(Construct),	FUNC(func_construct) },
	{ RED(Call),		FUNC(obj_call) },
	{ RED(Length),		Zero },
	{ RED(Args),		NULL },
	{ RED(Apply),		FUNC(func_apply) },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject array_obj = {
	0,
	{ RED(Construct),	FUNC(func_construct) },
	{ RED(Concat),		FUNC(array_concat) },
	{ RED(Join),		FUNC(array_join) },
	{ RED(Push),		FUNC(array_push) },
	{ RED(Pop),		FUNC(array_pop) },
	{ RED(Reverse),		FUNC(array_reverse) },
	{ RED(Shift),		FUNC(array_shift) },
	{ RED(Unshift),		FUNC(array_unshift) },
	{ RED(Slice),		FUNC(array_slice) },
	{ RED(Sort),		FUNC(array_sort) },
	{ RED(Splice),		FUNC(array_splice) }, 
	{ RED(Length),		Zero },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject error_obj = {
	0,

	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject scope_obj = {
	0,
	
	{ RED(This), global_obj },
	{ RED(Scope), NULL },	
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject argument_obj = {
	0,

	{ RED(Callee), NULL },
	{ RED(Length), ZERO },
	{ NULL, array_obj },
	{ NULL, NULL }
};

JObject file_obj = {
	0,

	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject math_obj = {
	0,
	{ RED(_E_),		FUNC(math_e) },
	{ RED(LN10),		FUNC(math_ln10) },
	{ RED(LN2),		FUNC(math_ln2) },
	{ RED(LOG2E),		FUNC(math_log2e) },
	{ RED(LOG10E),		FUNC(math_log10e) },
	{ RED(PI),		FUNC(math_pi) },
	{ RED(SQRT1_2),		FUNC(math_sqrt12) },
	{ RED(SQRT2),		FUNC(math_sqrt2) },
	{ RED(Abs),		FUNC(math_abs) },
	{ RED(Acos),		FUNC(math_acos) },
	{ RED(Asin),		FUNC(math_asin) },
	{ RED(Atan),		FUNC(math_atan) },
	{ RED(Atan2),		FUNC(math_atan2) },
	{ RED(Ceil),		FUNC(math_ceil) },
	{ RED(Cos),		FUNC(math_cos) },
	{ RED(Exp),		FUNC(math_exp) },
	{ RED(Floor),		FUNC(math_floor) },
	{ RED(Log),		FUNC(math_log) },
	{ RED(Max),		FUNC(math_max) },
	{ RED(Min),		FUNC(math_min) },
	{ RED(Pow),		FUNC(math_pow) },
	{ RED(Random),		FUNC(math_random) },
	{ RED(Round),		FUNC(math_round) },
	{ RED(Sin),		FUNC(math_sin) },
	{ RED(Sqrt),		FUNC(math_sqrt) },
	{ RED(Tan),		FUNC(math_tan) },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject date_obj = {
	0,

	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject socket_obj = {
	0,

	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject regexp_obj = {
	0,
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject xml_obj = {
	0,
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject global_obj = { 
	0,
	{ RED(Global), global_obj },
	{ RD(Object), object_obj },
	{ RD(String), string_obj },
	{ RD(Boolean), boolean_obj },
	{ RD(Integer), integer_obj },
	{ RD(Number), number_obj },
	{ RD(Error), error_obj },
	{ RD(Function), function_obj },
	{ RD(Date), date_obj },
	{ RD(File), file_obj },
	{ RD(Math), math_obj },
	{ RD(RegExp), regexp_obj },
	{ RD(Socket), socket_obj },
	{ RD(XML), xml_obj },
	{ NULL, object_obj },
	{ NULL, NULL }
};
