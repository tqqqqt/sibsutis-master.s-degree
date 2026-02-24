-- 1.1
create table teachers(t_id int primary key, t_name varchar(255), t_age int);

insert into teachers values(1,'t1',38);
insert into teachers values(2,'t2',75);
insert into teachers values(3,'t3',30);
insert into teachers values(4,'t4',42);

select * from teachers;

create table subjects(s_id int primary key, s_name varchar(255), t_id int references teachers(t_id), s_exam bool, s_hours int);

insert into subjects values(1,'math',1,true,108);
insert into subjects values(2,'bio',3,true,108);
insert into subjects values(3,'prog',4,false,130);
insert into subjects values(4,'sport',2,false,88);
insert into subjects values(5,'prog2',4,true,130);
insert into subjects values(6,'lang',3,false,24);

select * from subjects;

drop table subjects;
drop table teachers;

-- 1.2
create table students(sid int primary key, sname varchar(255));
insert into students values(1,'st1');
insert into students values(2,'st2');

create table subjects(sid int primary key, sname varchar(255));
insert into subjects values(1,'su1');
insert into subjects values(2,'su2');
insert into subjects values(3,'su3');

create table students_program(spid int primary key, st_id int references students(sid), su_id int references subjects(sid));
insert into students_program values(1,1,1);
insert into students_program values(2,1,3);
insert into students_program values(3,2,1);
insert into students_program values(4,2,2);

select * from students;
select * from subjects;
select * from students_program;

drop table students_program;
drop table students;
drop table subjects;