DROP TABLE author;
DROP TABLE book;
DROP TABLE format;
DROP TABLE author_seq;
DROP TABLE book_seq;
DROP TABLE format_seq;
DROP VIEW book_view;

CREATE SEQUENCE author_seq;
CREATE SEQUENCE book_seq;
CREATE SEQUENCE format_seq;

CREATE TABLE author ( id int primary key, forename varchar(40), surname varchar(40) );
CREATE TABLE book ( id int primary key, title varchar(40), price float, authorid int, formatid int );
CREATE TABLE format ( id int primary key, name varchar(20) );
CREATE VIEW book_view AS ( SELECT book.id AS id, book.title AS title,
book.price AS price, authorid, format.name AS format FROM book, author,
format WHERE book.authorid=author.id AND book.formatid=format.id );
CREATE VIEW author_view AS ( SELECT author.id AS id, author.forename || ' ' || author.surname AS name);

SELECT nextval( 'author_seq' );
SELECT nextval( 'author_seq' );
SELECT nextval( 'author_seq' );
INSERT INTO author VALUES ( 1, 'Philip K', 'Dick' );
INSERT INTO author VALUES ( 2, 'Robert', 'Heinlein' );
INSERT INTO author VALUES ( 3, 'Sarah', 'Paretsky' );

SELECT nextval( 'book_seq' );
SELECT nextval( 'book_seq' );
SELECT nextval( 'book_seq' );
SELECT nextval( 'book_seq' );
SELECT nextval( 'book_seq' );
SELECT nextval( 'book_seq' );
INSERT INTO book VALUES ( 1, 'The Man Who Japed', 6.99, 1, 2 );
INSERT INTO book VALUES ( 2, 'The Man in the High Castle', 9.99, 1, 1 );
INSERT INTO book VALUES ( 3, 'The Number of the Beast', 8.99, 2, 3 );
INSERT INTO book VALUES ( 4, 'Indemnity Only', 9.99, 3, 4 );
INSERT INTO book VALUES ( 5, 'Burn Marks', 9.99, 3, 1 );
INSERT INTO book VALUES ( 6, 'Deadlock', 9.99, 3, 1 );

SELECT nextval( 'format_seq' );
SELECT nextval( 'format_seq' );
SELECT nextval( 'format_seq' );
INSERT INTO format VALUES ( 1, 'Paperback' );
INSERT INTO format VALUES ( 2, 'Hardback' );
INSERT INTO format VALUES ( 3, 'Softback' );
