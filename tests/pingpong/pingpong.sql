-- create the pingpong database
-- should be run on postgres 6 or > database

drop sequence playerid_sequence;
drop sequence teamid_sequence;
drop sequence matchid_sequence;
drop sequence player2teamid_sequence;

drop table match;
drop table player2team;
drop table player;
drop table team;

create sequence playerid_sequence;
create table player
(id int4 primary key default nextval('playerid_sequence'), 
name varchar(50));

create sequence teamid_sequence;
create table team
(id int4 primary key default nextval('teamid_sequence'),
name varchar(50));

create sequence player2teamid_sequence;
create table player2team
(id int4 primary key default nextval('player2teamid_sequence'),
teamid int4 references team on delete cascade,
playerid int4 references player on delete cascade);

create sequence matchid_sequence;
create table match
(id int4 primary key default nextval('matchid_sequence'),
date date not null,
sets int4 not null,
winnerid int4 references team on delete cascade,
loserid int4 references team on delete cascade,
wins int4 not null,
losses int4 not null);



