PRAGMA user_version = 1;
PRAGMA foreign_keys = ON;

CREATE TABLE Configuration (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    label TEXT UNIQUE NOT NULL,
    name TEXT,
    value REAL
) STRICT;

CREATE TABLE Plant (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    label TEXT UNIQUE NOT NULL,
    capacity REAL NOT NULL DEFAULT 0
) STRICT;

CREATE TABLE Plant_vector_costs (
    id INTEGER,
    vector_index INTEGER NOT NULL,
    costs REAL NOT NULL,
    FOREIGN KEY(id) REFERENCES Plant(id) ON DELETE CASCADE ON UPDATE CASCADE,
    PRIMARY KEY (id, vector_index)
) STRICT;
