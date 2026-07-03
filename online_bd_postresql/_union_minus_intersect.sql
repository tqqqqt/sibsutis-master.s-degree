-- 1
select cname, city, ('Высокий рейтинг (' || rating || ')') as rate from cust where rating>=200
union
select cname, city, ('Низкий рейтинг (' || rating || ')') as rate from cust where rating<200;

-- 2
select cnum, cname from cust t1 where 1<(select count(*) from ord t2 where t1.cnum=t2.cnum)
union
select snum, sname from sal t1 where 1<(select count(*) from ord t2 where t1.snum=t2.snum);

-- 3
insert into sal values (1099, 'temp', 'temp', 0.12);
select snum from sal except select snum from ord;
delete from sal where snum=1099;

-- 4
insert into cust values (2099, 'Peel', 'temp', 100, 1001);
select cname from cust intersect select sname from sal;
delete from cust where cnum=2099;