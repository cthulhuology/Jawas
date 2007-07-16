// objects.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

JObject object_obj = {
	24,
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
	18,
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
	3,
	{ RED(Construct),	FUNC(bool_construct) },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject integer_obj = {
	3,
	{ RED(Construct),	FUNC(int_construct) },
	{ NULL, number_obj },
	{ NULL, NULL }
};

JObject number_obj = {
	10,
	{ RED(Construct),	FUNC(num_construct) },
	{ RED(isNaN),		FUNC(num_isNan) },
	{ RED(isFinite),	FUNC(num_isFinite) },
	{ RED(toFixed),		FUNC(num_to_fixed) },
	{ RED(toExponential),	FUNC(num_to_exp) },
	{ RED(toPrecision),	FUNC(num_to_precision) },
	{ RED(NaN),		NaN,
	{ RED(Infinity),	Infinity,
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject function_obj = {
	7,
	{ RED(Construct),	FUNC(func_construct) },
	{ RED(Call),		FUNC(obj_call) },
	{ RED(Length),		Zero },
	{ RED(Args),		Empty },
	{ RED(Apply),		FUNC(func_apply) },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject array_obj = {
	14,
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
	4,
	{ RED(Name), Error },
	{ RED(Message), Empty },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject scope_obj = {
	4,
	{ RED(This), global_obj },
	{ RED(Scope), NULL },	
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject argument_obj = {
	4,
	{ RED(Callee), NULL },
	{ RED(Length), ZERO },
	{ NULL, array_obj },
	{ NULL, NULL }
};

JObject math_obj = {
	28,
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
	28,
	{ RED(MakeDay),		FUNC(date_make_day) },
	{ RED(MakeTime),	FUNC(date_make_time) },
	{ RED(MakeDate),	FUNC(date_make_date) },
	{ RED(TimeClip),	FUNC(date_time_clip) },
	{ RED(Parse),		FUNC(date_parse) },
	{ RED(UTC),		FUNC(date_utc) },
	{ RED(toDateString),	FUNC(date_to_date_string) },
	{ RED(toTimeString),	FUNC(date_to_time_string) },
	{ RED(getTime),		FUNC(date_get_time) },
	{ RED(getFullYear),	FUNC(date_get_full_year) },
	{ RED(getMonth),	FUNC(date_get_month) },
	{ RED(getDate),		FUNC(date_get_date) },
	{ RED(getDay),		FUNC(date_get_day) },
	{ RED(getHours),	FUNC(date_get_hours) },
	{ RED(getMinutes),	FUNC(date_get_minutes) },
	{ RED(getSeconds),	FUNC(date_get_seconds) },
	{ RED(getMilliseconds),	FUNC(date_get_milliseconds) },
	{ RED(getTimezone),	FUNC(date_get_timezone) },
	{ RED(setTime),		FUNC(date_set_time) },
	{ RED(setMilliseconds),	FUNC(date_set_milliseconds) },
	{ RED(setSeconds),	FUNC(date_set_seconds) },
	{ RED(setMinutes),	FUNC(date_set_minutes) },
	{ RED(setHours),	FUNC(date_set_hours) },
	{ RED(setDate),		FUNC(date_set_date) },
	{ RED(setMonth),	FUNC(date_set_month) },
	{ RED(setFullYear),	FUNC(date_set_full_year) },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject file_obj = {
	7,
	{ RED(Open),		FUNC(file_open) },
	{ RED(Close),		FUNC(file_close) },
	{ RED(Read),		FUNC(file_read) },
	{ RED(Write),		FUNC(file_write) },
	{ RED(Seek),		FUNC(file_seek) },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject socket_obj = {
	9,
	{ RED(Connect),		FUNC(socket_connect) },
	{ RED(Listen),		FUNC(socket_listen) },
	{ RED(Accept),		FUNC(socket_accept) },
	{ RED(Bind),		FUNC(socket_bind) },
	{ RED(Shutdown),	FUNC(socket_shutdown) },
	{ RED(Read),		FUNC(socket_read) },
	{ RED(Write),		FUNC(socket_write) },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject regexp_obj = {
	9,
	{ RED(global),		False },
	{ RED(source),		False },
	{ RED(ignoreCase),	False },
	{ RED(multiline),	False },
	{ RED(lastIndex),	False },
	{ RED(Exec),		FUNC(regexp_exec) },
	{ RED(Test),		FUNC(tegexp_test) },
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject xml_obj = {
	2,
	{ NULL, object_obj },
	{ NULL, NULL }
};

JObject global_obj = {
	16,
	{ RED(Global), global_obj },
	{ RD(Object), object_obj },
	{ RD(String), string_obj },
	{ RD(Boolean), boolean_obj },
	{ RD(Integer), integer_obj },
	{ RD(Number), number_obj },
	{ RD(Error), error_obj },
	{ RD(Function), function_obj },
	{ RD(Date), date_obj },
	{ RD(Math), math_obj },
	{ RD(RegExp), regexp_obj },
	{ RD(File), file_obj },
	{ RD(Socket), socket_obj },
	{ RD(XML), xml_obj },
	{ NULL, object_obj },
	{ NULL, NULL }
};


