
DROP TABLE news;

CREATE TABLE news (
	id serial,
	author serial,
	newsdate date,
	topic varchar2,
	intro varchar2,
	body varchar2,
	PRIMARY KEY(id)
);

DROP TABLE ftp;

CREATE TABLE ftp (
	id serial,
	url varchar2,
	desc varchar2,
	PRIMARY KEY(id)
);

DROP TABLE ftpcontent;

CREATE TABLE ftpcontent (
	id serial,
	ftp serial not null,
	url varchar2,
	chksum varchar2,
	PRIMARY KEY(id)
);

DROP TABLE packetonftp;

CREATE TABLE packetonftp (
	id serial,
	name varchar2,
	ftpcontent serial,
	PRIMARY KEY(id)
);