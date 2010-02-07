-- lua

-- returns a list of functions in
create or replace function functions_in(nsp text) returns setof text as $$
	select proc.proname::text from pg_proc proc
		join pg_namespace namesp on proc.pronamespace = namesp.oid
		where namesp.nspname = $1;
$$ language sql;

create or replace function guid() returns bigint as $$
	select nextval('guid_seq');
$$ language sql;
