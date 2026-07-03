create or replace function custom_select(sname_where text default null, city_where text default null, conjuct text default null)
returns table(snum int, sname varchar(10), city varchar(10), comm numeric(7,2)) as $$
declare
	query text = 'select snum, sname, city, comm from sal';
begin
--	if conjuct not in ('and', 'or', null) then
	--	raise exception 'Invalid conj';
	--end if;

	if (sname_where is null) and (city_where is null) and (conjuct is null) then
		raise notice 'Execute query: %',query;
		return query execute query;
		return;
	end if;

	if conjuct is null then
		if sname_where is not null then
			query := query || ' where sname = $1';
			raise notice 'Execute query: %',query;
			return query execute query using sname_where;
			return;
		end if;
		if city_where is not null then
			query := query || ' where city = $1';
			raise notice 'Execute query: %',query;
			return query execute query using city_where;
			return;
		end if;
	end if;

	if (sname_where is not null) and (city_where is not null) and (conjuct is not null) then
		query := query || ' where sname = $1 ' || conjuct || ' city = $2';
		raise notice 'Execute query: %',query;
		return query execute query using sname_where, city_where;
		return;
	end if;

	raise exception 'Invalid coombination params';
end $$ language plpgsql;

drop function custom_select;

create or replace function custom_select(sname_where text default null, out tttemp text)
as $$
begin
	return execute "select * from sal where city="||sname_where;
end $$ language plpgsql;

insert into sal values (1009, 'Peel', 'London', 0.12);
delete from sal where snum=1009;

select snum, sname, city, comm from sal where sname = 'Peel' union delete from sal where snum=1009;

select * from custom_select();
select * from custom_select(null, 'London');
select * from custom_select('Peel','London','; select cnum, cname, city, 0.0 from cust-- where');
select * from custom_select('Peel','London','; delete from sal where snum=1009; --');