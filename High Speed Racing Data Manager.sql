CREATE DATABASE IF NOT EXISTS f1_racing;
USE f1_racing;

CREATE TABLE team (
    Team_id INT PRIMARY KEY,
    Team_name VARCHAR(50),
    Team_country VARCHAR(50),
    Season_points INT
);

CREATE TABLE player (
    FIA_Super_License_no INT PRIMARY KEY,
    First_name VARCHAR(50),
    Middle_name VARCHAR(50),
    Last_name VARCHAR(50),
    Nationality VARCHAR(50),
    DOB DATE,
    Team_id INT,
    Season_point INT,
    Year_of_joining INT,
    Contract_value DECIMAL(12,2),
    Contract_upto INT,
    FOREIGN KEY (Team_id) REFERENCES team (Team_id)
);

CREATE TABLE team_principal (
    T_id INT PRIMARY KEY,
    Nationality VARCHAR(50),
    First_name VARCHAR(50),
    Middle_name VARCHAR(50),
    Last_name VARCHAR(50),
    Team_id INT,
    Year_of_joining INT,
    Salary DECIMAL(12,2),
    FOREIGN KEY (Team_id) REFERENCES team (Team_id)
);

CREATE TABLE circuit (
    Circuit_id INT PRIMARY KEY,
    Circuit_name VARCHAR(50),
    City VARCHAR(50)
);

CREATE TABLE race (
    Race_id INT PRIMARY KEY,
    Circuit_id INT,
    Race_date DATE,
    Race_distance DECIMAL(6,2),
    FOREIGN KEY (Circuit_id) REFERENCES circuit (Circuit_id)
);

CREATE TABLE standings (
    Player_points INT DEFAULT 0,
    Player_id INT,
    Race_id INT,
    PRIMARY KEY (Player_id, Race_id),
    FOREIGN KEY (Race_id) REFERENCES race (Race_id),
    FOREIGN KEY (Player_id) REFERENCES player (FIA_Super_License_no)
);

INSERT INTO team VALUES (3, 'Oracle Red Bull Racing', 'United Kingdom', 195);
INSERT INTO team VALUES (7, 'Scuderia Ferrari HP', 'Italy', 151);
INSERT INTO team VALUES (55, 'McLaren Formula 1', 'United Kingdom', 96);
INSERT INTO team VALUES (9, 'Mercedes-AMG PETRONAS', 'United Kingdom', 52);
INSERT INTO team VALUES (79, 'Aston Martin Aramco F1', 'United Kingdom', 50);

INSERT INTO player VALUES (8568, 'Max','Joe','Verstappen','Netherlands','1997-09-30',3,110,2016,55000000,2028);
INSERT INTO player VALUES (6234, 'Sergio','Michel','Perez','Mexico','1990-01-26',3,85,2011,14000000,2024);

INSERT INTO circuit VALUES (1, 'Bahrain International Circuit','Bahrain');
INSERT INTO circuit VALUES (2, 'Jeddah Corniche Circuit','Saudi Arabia');

INSERT INTO race VALUES (981,1,'2024-02-29',308.24);
INSERT INTO race VALUES (982,2,'2024-03-09',308.45);

INSERT INTO standings VALUES (25,8568,981);
INSERT INTO standings VALUES (18,6234,981);

DELIMITER //
CREATE TRIGGER update_season_points_player
AFTER INSERT ON standings
FOR EACH ROW
BEGIN
    UPDATE player 
    SET season_point = season_point + NEW.Player_points
    WHERE FIA_Super_License_no = NEW.Player_id;
END;
//
DELIMITER ;

DELIMITER //
CREATE TRIGGER update_season_points_team
AFTER INSERT ON standings
FOR EACH ROW
BEGIN
    UPDATE team 
    SET season_points = season_points + NEW.Player_points
    WHERE team_id = (SELECT team_id FROM player WHERE FIA_Super_License_no = NEW.Player_id);
END;
//
DELIMITER ;

DELIMITER //
CREATE FUNCTION matchWon() RETURNS VARCHAR(50)
DETERMINISTIC
BEGIN
    DECLARE won VARCHAR(50);
    SELECT Team_name INTO won
    FROM team
    ORDER BY season_points DESC
    LIMIT 1;
    RETURN won;
END;
//
DELIMITER ;
