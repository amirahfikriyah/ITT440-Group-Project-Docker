CREATE DATABASE IF NOT EXISTS itt440;
USE itt440;

CREATE TABLE IF NOT EXISTS scores (
  user VARCHAR(64) PRIMARY KEY,
  points INT NOT NULL DEFAULT 0,
  datetime_stamp DATETIME NOT NULL
);

INSERT INTO scores (user, points, datetime_stamp)
VALUES
  ('python_user', 0, NOW()),
  ('c_user', 0, NOW())
ON DUPLICATE KEY UPDATE user=user;
