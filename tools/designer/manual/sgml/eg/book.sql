drop table author;
drop table book;
drop table format;
drop table author_seq;
drop table book_seq;
drop table format_seq;
drop view book_view;

create sequence author_seq;
create sequence book_seq;
create sequence format_seq;

create table author ( id int primary key, forename varchar(40), surname varchar(40) );
create table book ( id int primary key, title varchar(40), price float, authorid int, formatid int );
create table format ( id int primary key, name varchar(20) );
create view book_view as ( select book.id as id, book.title as title,
book.price as price, authorid, format.name as format from book, author,
format where book.authorid=author.id and book.formatid=format.id );
create view author_view as ( select author.id as id, author.forename || ' ' || author.surname as name);

select nextval( 'author_seq' );
select nextval( 'author_seq' );
select nextval( 'author_seq' );
insert into author values ( 1, 'Philip K', 'Dick' );
insert into author values ( 2, 'Robert', 'Heinlein' );
insert into author values ( 3, 'Sarah', 'Paretsky' );

select nextval( 'book_seq' );
select nextval( 'book_seq' );
select nextval( 'book_seq' );
select nextval( 'book_seq' );
select nextval( 'book_seq' );
select nextval( 'book_seq' );
insert into book values ( 1, 'The Man Who Japed', 6.99, 1, 2 );
insert into book values ( 2, 'The Man in the High Castle', 9.99, 1, 1 );
insert into book values ( 3, 'The Number of the Beast', 8.99, 2, 3 );
insert into book values ( 4, 'Indemnity Only', 9.99, 3, 4 );
insert into book values ( 5, 'Burn Marks', 9.99, 3, 1 );
insert into book values ( 6, 'Deadlock', 9.99, 3, 1 );

select nextval( 'format_seq' );
select nextval( 'format_seq' );
select nextval( 'format_seq' );
select nextval( 'format_seq' );
insert into format values ( 1, 'Paperback' );
insert into format values ( 2, 'Hardback' );
insert into format values ( 3, 'Softback' );
insert into format values ( 4, 'Ringbound' );
