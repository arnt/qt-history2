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

create table authors
(id numeric foreign key,
 name char(30));

insert into authors values (1, 'Selma Lagerlöf');
insert into authors values (2, 'Astrid Lindgren');

create table publishers
(id numeric,
 address char(40));

insert into publishers values (1, 'Norstedt');
insert into publishers values (3, 'Prismas');
