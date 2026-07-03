create table shooter_list(s_id int primary key, s_name varchar(255), team varchar(255), pos varchar(255), goals int, penalty int, play_time int, matches int);
drop table shooter_list;

INSERT INTO shooter_list(s_id, s_name, team, pos, goals, penalty, play_time, matches) VALUES
(1, 'James Rodríguez', 'Colombia', 'FW', 6, 0, 520, 6),
(2, 'Thomas Müller', 'Germany', 'FW', 5, 1, 610, 7),
(3, 'Lionel Messi', 'Argentina', 'FW', 4, 1, 590, 7),
(4, 'Neymar', 'Brazil', 'FW', 4, 1, 450, 5),
(5, 'Robin van Persie', 'Netherlands', 'FW', 4, 1, 530, 6),
(6, 'Arjen Robben', 'Netherlands', 'FW', 3, 0, 480, 6),
(7, 'André Schürrle', 'Germany', 'FW', 3, 0, 360, 6),
(8, 'Clint Dempsey', 'USA', 'FW', 3, 0, 340, 4),
(9, 'Karim Benzema', 'France', 'FW', 3, 0, 380, 4),
(10, 'Miroslav Klose', 'Germany', 'FW', 2, 0, 470, 6),
(11, 'Luis Suárez', 'Uruguay', 'FW', 3, 1, 420, 6),
(12, 'Tim Cahill', 'Australia', 'FW', 2, 0, 260, 3),
(13, 'Xherdan Shaqiri', 'Switzerland', 'FW', 2, 0, 340, 4),
(14, 'Diego Costa', 'Spain', 'FW', 2, 0, 210, 3),
(15, 'Gonzalo Higuaín', 'Argentina', 'FW', 2, 0, 430, 6),
(16, 'Jackson Martínez', 'Colombia', 'FW', 1, 0, 220, 3),
(17, 'Javier Hernández', 'Mexico', 'FW', 2, 1, 260, 4),
(18, 'Edinson Cavani', 'Uruguay', 'FW', 1, 1, 320, 4),
(19, 'Sergio Agüero', 'Argentina', 'FW', 1, 0, 230, 3),
(20, 'Eden Hazard', 'Belgium', 'MF', 2, 1, 380, 5),
(21, 'Romelu Lukaku', 'Belgium', 'FW', 1, 0, 280, 3),
(22, 'Jozy Altidore', 'USA', 'FW', 1, 0, 200, 2),
(23, 'Daniel Sturridge', 'England', 'FW', 1, 0, 220, 3),
(24, 'Steven Gerrard', 'England', 'MF', 1, 1, 350, 4),
(25, 'Philippe Coutinho', 'Brazil', 'MF', 1, 0, 340, 4),
(26, 'Marco Reus', 'Germany', 'FW', 1, 0, 160, 2),
(27, 'David Silva', 'Spain', 'MF', 1, 0, 300, 4),
(28, 'Joel Campbell', 'Costa Rica', 'FW', 1, 0, 290, 5),
(29, 'Bryan Ruiz', 'Costa Rica', 'MF', 1, 1, 410, 5),
(30, 'Alexis Sánchez', 'Chile', 'FW', 1, 0, 370, 4),
(31, 'Arturo Vidal', 'Chile', 'MF', 2, 0, 310, 4),
(32, 'Mario Mandžukić', 'Croatia', 'FW', 1, 0, 300, 4),
(33, 'Xabi Alonso', 'Spain', 'MF', 1, 0, 280, 4),
(34, 'André Ayew', 'Ghana', 'FW', 1, 0, 220, 3),
(35, 'Asamoah Gyan', 'Ghana', 'FW', 1, 0, 270, 3),
(36, 'Shinji Okazaki', 'Japan', 'FW', 1, 0, 300, 4),
(37, 'Keisuke Honda', 'Japan', 'MF', 0, 0, 360, 4),
(38, 'Gervinho', 'Ivory Coast', 'FW', 1, 0, 210, 3),
(39, 'Mario Götze', 'Germany', 'MF', 0, 0, 420, 7),
(40, 'Paulinho', 'Brazil', 'MF', 2, 1, 240, 3);

select * from shooter_list;


-- 1
select s_name, pos, sum(penalty) over (partition by pos) from shooter_list;

-- 2
select team, s_name, goals from (select s_name, team, goals, row_number() over (partition by team order by goals desc) as pos from shooter_list) where pos<4;

-- 3
select s_name, team, goals, (count(*) over (partition by team order by goals desc range between unbounded preceding and current row))-1 as res from shooter_list;

-- 4
select s_name, team, matches, (count(*) over (partition by team order by matches desc range between current row and unbounded following))-1 as res from shooter_list; 

-- 5
select s_name, (goals*1.0/play_time) as ef, ((goals*1.0/play_time)-lag((goals*1.0/play_time)) over (order by (goals*1.0/play_time))) as cur_m_prev from shooter_list;

-- 6
select s_name, team, play_time, goals, avg(play_time) over (partition by team) from shooter_list where goals>0;

-- 7
select s_name, team, goals, play_time, avg(play_time) filter (where rnk<=2) over (partition by team) 
from (select s_name, team, goals, play_time, rank() over (partition by team order by goals desc) as rnk from shooter_list);

-- 8
select s_name, team, goals, play_time, avg(play_time) over (partition by team order by goals desc range between current row and current row) as res from shooter_list;
