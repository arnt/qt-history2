--    LocalSQL
--
--    Copyright (C) 2001 Trolltech AS
--
--    Contact:
--            Dave Berton (db@trolltech.com)
--            Jasmin Blanchette (jasmin@trolltech.com)
--
--    This library is free software; you can redistribute it and/or
--    modify it under the terms of the GNU Lesser General Public
--    License as published by the Free Software Foundation; either
--    version 2.1 of the License, or (at your option) any later version.
--
--    This library is distributed in the hope that it will be useful,
--    but WITHOUT ANY WARRANTY; without even the implied warranty of
--    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--    Lesser General Public License for more details.
--
--    You should have received a copy of the GNU Lesser General Public
--    License along with this library; if not, write to the Free Software
--    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--

drop table alltypes;

create table alltypes (
n1 int primary key,
n2 numeric( 19, 3 ),
c1 char( 50 ),
c2 char( 10 ),
d1 date,
c5673 char(2)
);

create index alltypesindex_1 on alltypes ( n1, n2 ); 
create index alltypesindex_2 on alltypes ( c1 ); 
create index alltypesindex_3 on alltypes ( d1 ); 

insert into alltypes values( 1, 12, 'latin1: ικλε ”™', 'more', '2001-01-11'); 
insert into alltypes values( 91, 12, 'csdfsdf', 'more', '2000-01-01');
insert into alltypes values( 92, 22, 'csdfsdf', 'more', '2000-01-01');
insert into alltypes values( 93, 32, 'xsdfsdf', 'more', '2000-01-01');
insert into alltypes values( 94, 42, 'csdfsdf', 'more', '2000-01-01');
insert into alltypes values( 95, 52, 'csdfsdf', 'more', '2000-01-01');
insert into alltypes values( 96, 62, 'xsdfsdf', 'more', '2000-01-01');
insert into alltypes values( 97, 72, 'swdfsdf', 'more', '2000-01-01');
insert into alltypes values( 98, 82, 'sdsddfsdf', 'more', '2000-01-01');
insert into alltypes values( 99, 92, 'sfdfsdf', 'more', '2000-01-01');
--insert into alltypes values( 100, NULL, NULL, NULL, NULL );

update alltypes set c2 = 'updated' where n1 = 1;
update alltypes set c2 = 'updated1' where n1 = 1;
update alltypes set c2 = 'updated2' where n1 = 1;
update alltypes set c2 = 'updated3' where n1 = 1;
update alltypes set c2 = 'updated4' where n1 = 1;
update alltypes set n2 = 999 where n1 = 1;
update alltypes set n2 = NULL where n2 = 12;
update alltypes set d1 = '1972-04-18' where n1 = 1;

-- actually change a primary key
update alltypes set n1 = 999 where n1 = 1;

-- change a whole bunch of records
update alltypes set c1 = 'mass update' where n1 > 1;

-- complex where clause
update alltypes set c1 = 'complex update' where n1 = 91 OR ( n1+n2 > 189 );


-- some select statements
select * from alltypes;
select count(n1) from alltypes;
select n1,n2 from alltypes group by n1, n2;
select n1, alltypes.* from alltypes;
--select a.n1 from alltypes a, alltypes b ## currently fails

-- another table
create table another
(id int primary key,
linkid int);
create index another_link_idx on another(linkid);
insert into another values (1,99);
insert into another values (1,NULL);

-- some joins
select a.linkid, b.n1 from another a, alltypes b
       where a.linkid=b.n1;

--delete from alltypes;
