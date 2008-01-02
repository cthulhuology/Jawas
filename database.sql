-- database.sql
--
-- Copyright (C) 2007 David J. Goehrig
-- All Rights Reserved
--

drop sequence guid_seq;
create sequence guid_seq;

drop table object;
create table object (
	id		bigint,
	data		text
);
grant all on object to jawas;

drop table scripts;
create table scripts (
	id		bigint,
	site		varchar,
	category	varchar default 'None',
	class		varchar default 'None',
	name		varchar not null,
	func		text,
	active		bool default true
);
grant all on scripts to jawas;


