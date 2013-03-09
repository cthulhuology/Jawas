-- database.sql
--
-- Copyright (C) 2007 David J. Goehrig
-- All Rights Reserved
--

drop user if exists jawas;
create user jawas with login;

drop sequence if exists guid_seq;
create sequence guid_seq;

drop table if exists object;
create table object (
	id		bigint,
	data		text
);
grant all on object to jawas;

drop table if exists scripts;
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


