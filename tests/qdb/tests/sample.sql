create table alltypes (
n1 numeric,
n2 numeric( 10, 2 ),
c1 char( 255 ),
d1 date
);

create unique index alltypesindex on alltypes ( n1 );
create index alltypesindex on alltypes ( n1, n2 );
create index alltypesindex on alltypes ( n1, n2, c1 );
create index alltypesindex on alltypes ( n1, n2, c1, d1 );
