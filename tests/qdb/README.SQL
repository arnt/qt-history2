  LocalSQL

  Copyright (C) 2001 Trolltech AS

  Contact:
	 Dave Berton (db@trolltech.com)
	 Jasmin Blanchette (jasmin@trolltech.com)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


This file outlines the level of SQL supported by LocalSQL.

Data Types
----------

CHAR, VARCHAR
INT, INTEGER
DECIMAL, FLOAT, DOUBLE, NUMERIC
DATE

SQL Statements
--------------

The following SQL-schema statements are supported:
	<table definition>
	<drop table statement>

The following SQL-data statements are supported:
	<select statement>
	<insert statement>
	<update statement>
	<delete statement>

The following set functions are supported:
	AVG
	MAX
	MIN
	SUM
	COUNT
	DISTINCT ##?

The following string value functions are supported:
	UPPER
	LOWER
	LENGTH, CHARACTER_LENGTH
	SUBSTRING

The following datetime value functions are supported:
	CURRENT_DATE

The following predicates are supported:
	<comparison>
	<between>
	<like>
	<null>

NULL values are supported.

SELECT statements are supported, with the following form:
	<table expression>
		<from clause>
		[ <where clause> ]
		[ <group by clause> ]
		[ <having clause> ]

Multiple table SELECTs are supported.

Note on constraints
-------------------

Unique column constraints are supported, for example:

	create table t
	(id int unique);

However, NOT NULL, PRIMARY KEY, FOREIGN KEY and CHECK contraints are
not fully supported.  For example:

	create table t
	(id int not null);

The above issues a warning, but creates the table anyway.

	create table t
	(id int primary key);

The above issues a warning, but creates a unique index anyway.

	create table t
	(id int foreign key);

The above issues a warning, but creates an index anyway.

	create table t
	(id int check ...);

The above issues a warning, but creates the table anyway.


Note on strings
---------------
Unicode ##?

Stuff not supported
-------------------

DATETIME and INTERVAL data types are not supported
Referential integrity constraints are not supported.
UNIONs are not supported.
JOINs are not supported.
INTERSECTs are not supported.
Cast specifications are not supported.
Numeric value functions are not supported.
SQL-transaction statements are not supported.
SQL-connection statements are not supported.
SQL-session statements are not supported.
SQL-dynamic statements are not supported.
SQL-procedure statements are not supported.
Language clauses are not supported.
Cursors are not supported.
User priviledges (GRANT, REVOKE, etc) are not supported.
Assertions are not supported.
Anything to do with translations is not supported.
Anything to do with intervals is not supported.
Anything to do with character sets is not supported.
Anything to do with collating is not supported.
Anything to do with dynamic SQL (PREPARE, EXECUTE, etc.) is not supported.
Anything to do with diagnostic management is not supported.
Anything to do with information schemas is not supported.
DEFAULT clauses for CREATE TABLE and INSERT statements are not supported.
ALTER TABLE is not supported.
ADD|DROP TABLE CONSTRAINT is not supported.
VIEWs are not supported.
DOMAINs are not supported.
Temporary tables are not supported.








