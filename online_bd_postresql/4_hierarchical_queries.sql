CREATE TABLE CORP (CRP_ID int primary key, CRP_PID int, CRP_NAME VARCHAR(20), CRP_WORTH int);

insert into CORP(CRP_ID,CRP_PID,CRP_NAME,CRP_WORTH) values (1,null,'MainFactory',900),
 (2,null,'ImportantBank',900), (3,1,'AutoFactory',700),
 (4,1,'DevFactory',500), (5,2,'SubBank',300),
 (6,2,'UnderBank',500), (7,4,'SomeCarFactory',400),
 (8,4,'AnotherCarFactory',400), (9,5,'MicroBank',100);

drop table CORP;
select * from CORP;


----
with recursive temp1 (crp_id, crp_pid, crp_name, crp_worth, root) as 
(
	select t1.crp_id, t1.crp_pid, t1.crp_name, t1.crp_worth, t1.crp_id as root
	from CORP t1
	union
	select t2.crp_id, t2.crp_pid, t2.crp_name, t2.crp_worth, temp1.root
	from CORP t2
	join temp1 on (t2.crp_pid=temp1.crp_id)
)
select crp_id, crp_name, res from
(
	select *, rank() over (order by res desc) as rnk from 
	(
		select t1.crp_id, t1.crp_name, sum(t2.crp_worth) as res 
		from corp t1
		join temp1 t2 on t1.crp_id=t2.root
		group by t1.crp_id, t1.crp_name
		limit 100
	)
) where rnk=1