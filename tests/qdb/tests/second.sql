insert into authors
values(7, 'Bar');

select id, name
from authors
order by id desc, name desc;
