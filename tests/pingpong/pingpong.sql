-- create the pingpong database
-- should be run on postgres 6 or > database

create table player
(id int4 primary key, 
name varchar(50));

create table team
(id int4 primary key,
name varchar(50));

create table player2team
(id int4 primary key,
teamid int4 references team,
playerid int4 references player);

create table match
(id int4 primary key,
date date not null,
sets int4 not null,
winnerid int4 references team,
loserid int4 references team,
wins int4 not null,
losses int4 not null);



