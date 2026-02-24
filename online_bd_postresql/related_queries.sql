-- соотнесеные подзапросы
select cname, cnum from cust a where 0 = (select count(*) from cust b where a.city=b.city and a.rating<b.rating);

select sal.snum, sal.sname from sal join cust on sal.city=cust.city and sal.snum!=cust.snum group by sal.snum;
select * from sal a where 0 < (select count(*) from cust b where a.city=b.city and a.snum!=b.snum);

-- предикат exists
select * from sal a where exists (select 1 from cust b where a.snum=b.snum and b.rating=300);

select * from cust a where exists (select 1 from ord b where a.snum=b.snum and a.cnum!=b.cnum);

-- предикаты any, some, all
select * from cust a where rating > any (select rating from cust b where b.snum=1002);

select * from sal a where a.city != all (select city from cust b where a.snum=b.snum);

select * from ord a where a.amt > any (select amt from ord where cnum in (select cnum from cust where city='London'));

-- внешние соединения
select sal.snum, sal.sname, cust.cnum, cust.cname from sal left join cust on sal.snum=cust.snum and cust.rating>100;

select cust.cnum, cust.cname, sal.snum, sal.sname from cust left join sal on cust.snum=sal.snum and (sal.city='London' or sal.city='Barcelona');

select * from cust;
insert into cust values(2009,'Temp','Rome',100,NULL);
select sal.snum, sal.sname, cust.cnum, cust.cname from sal full outer join cust on sal.snum=cust.snum;
delete from cust where cnum=2009;

-- контрольные вопросы (вариант 1)
select cnum+1 as cnum from cust a where cnum>2003 and 0 = (select count(*) from cust b where b.cnum>a.cnum);
select max(a.cnum)+1 as cnum from cust a full outer join cust b on a.cnum+1=b.cnum where b.cnum is null;

select * from cust a where exists (select 1 from cust b where a.cnum!=b.cnum and a.city=b.city);

select * from cust a where a.rating > any (select rating from cust b where a.city!=b.city);

--
select * from cust;
select min(cnum)+1 as cnum from cust a where cnum>2003 and 0 = (select count(*) from cust b where a.cnum=b.cnum-1);