DROP TABLE source, news;

CREATE TABLE source (
  id int unsigned NOT NULL auto_increment,
  url text not null, 
  descr text,
  date datetime not null,
  PRIMARY KEY (id)
);

CREATE TABLE news (
  id bigint unsigned NOT NULL auto_increment,
  sourceid int unsigned NOT NULL, 
  author varchar(255), 
  date datetime not null,
  topic text not null,
  intro text,
  static enum ('n','y') default 'n',
  PRIMARY KEY (id),
  FOREIGN KEY (sourceid) REFERENCES source (id)
);

create index ref_nsid on news (sourceid asc);
