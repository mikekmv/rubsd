
DROP SEQUENCE ftp_id_seq;
DROP SEQUENCE ftpcontent_ftp_seq;
DROP SEQUENCE news_author_seq;
DROP SEQUENCE packetonftp_ftpcontent_seq;

DROP TABLE news;

CREATE TABLE news (
	id serial,
	author serial,
	newsdate date,
	topic text,
	intro text,
	body text,
	PRIMARY KEY(id)
);

DROP TABLE ftp;

CREATE TABLE ftp (
	id serial,
	url text,
	PRIMARY KEY(id)
);

DROP TABLE ftpcontent;

CREATE TABLE ftpcontent (
	id serial,
	ftp serial not null,
	url text,
	chksum text,
	PRIMARY KEY(id)
);

DROP TABLE packetonftp;

CREATE TABLE packetonftp (
	id serial,
	name text,
	ftpcontent serial,
	PRIMARY KEY(id)
);
