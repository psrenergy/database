PRAGMA user_version = 1;
PRAGMA foreign_keys = ON;

CREATE TABLE Configuration (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    label TEXT UNIQUE NOT NULL,
    value1 REAL NOT NULL DEFAULT 100,
    enum1 TEXT NOT NULL DEFAULT 'A' CHECK(enum1 IN ('A', 'B', 'C'))
) STRICT;

CREATE TABLE Resource (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    label TEXT UNIQUE NOT NULL,
    type TEXT NOT NULL DEFAULT "D" CHECK(type IN ('D', 'E', 'F'))
) STRICT;

CREATE TABLE Resource_set_some_group (
    id INTEGER, 
    some_value_1 REAL NOT NULL,
    some_value_2 REAL,
    FOREIGN KEY(id) REFERENCES Resource(id) ON DELETE CASCADE ON UPDATE CASCADE,
    UNIQUE (id, some_value_1, some_value_2)
) STRICT;