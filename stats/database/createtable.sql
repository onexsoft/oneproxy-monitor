create table user(
	hashcode int, //hash(caddr+cport+saddr+sport+conndatetime)
	caddr varchar(20), //client ip address
	cport int, //client port
	saddr varchar(20), //server ip address
	sport int, //server port
	conndatetime datetime, //connect date time
	sendata bigint, //the bytes of send data to server
	recvdata bigint //the bytes of recv data from server
);

create table sql(
	hashcode int, //sql hashcode hash(sql)
	sql text,
	rownum int, //the sql query result
	querytime time,
	tabs int, //the table of sql
	exec int, //the number of sql execute.
	trans int,//the sql execute in translation
	fails int,//the number of sql execute failes
	recvsize int//the result's size of execute sql
);

create table user_sql_rel(
	userhc int,
	sqlhc int,
	exectime datetime
);

create table sqltable(
	hashcode int, //table name hashcode
	tabname text, //table name
);

create table sql_table_rel(
	sqlhc int, //sql hash code
	tabhc int, //table hash code
	//sqlhc and tabhc is unique.
);

create table transinfo(
	hashcode int,
	exectime datatime,
	starttime datetime,
	endtime datetime
);

create table trans_sql_rel(
	transhc int, //transinfo hashcode
	sqlhc int, //sql hash code
	//transhc and sqlhc is unique.
);
