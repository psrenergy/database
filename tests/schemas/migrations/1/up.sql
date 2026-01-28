-- create_first_snapshot
PRAGMA user_version = 1;
PRAGMA foreign_keys = ON;

CREATE TABLE Configuration (
    id INTEGER PRIMARY KEY,
    label TEXT UNIQUE NOT NULL
) STRICT;

CREATE TABLE Test1 (
    id INTEGER PRIMARY KEY,
    label TEXT UNIQUE NOT NULL,
    name TEXT
) STRICT;

CREATE TABLE Test2 (
    id INTEGER PRIMARY KEY,
    label TEXT UNIQUE NOT NULL,
    capacity INTEGER,
    some_coefficient REAL
) STRICT;