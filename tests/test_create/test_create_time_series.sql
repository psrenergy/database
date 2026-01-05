PRAGMA user_version = 1;
PRAGMA foreign_keys = ON;

CREATE TABLE Configuration (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    label TEXT UNIQUE NOT NULL,
    value1 REAL NOT NULL DEFAULT 100
) STRICT;

CREATE TABLE Plant (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    label TEXT UNIQUE NOT NULL,
    capacity REAL NOT NULL DEFAULT 0
) STRICT;

CREATE TABLE Plant_time_series_generation (
    id INTEGER,
    date_time TEXT NOT NULL,
    block INTEGER NOT NULL,
    generation REAL,
    FOREIGN KEY(id) REFERENCES Plant(id) ON DELETE CASCADE ON UPDATE CASCADE,
    PRIMARY KEY (id, date_time, block)
) STRICT;

CREATE TABLE Plant_time_series_prices (
    id INTEGER,
    date_time TEXT NOT NULL,
    segment INTEGER NOT NULL,
    price REAL,
    quantity REAL,
    FOREIGN KEY(id) REFERENCES Plant(id) ON DELETE CASCADE ON UPDATE CASCADE,
    PRIMARY KEY (id, date_time, segment)
) STRICT;

CREATE TABLE Resource (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    label TEXT UNIQUE NOT NULL
) STRICT;

CREATE TABLE Resource_time_series_availability (
    id INTEGER,
    date_time TEXT NOT NULL,
    value REAL,
    FOREIGN KEY(id) REFERENCES Resource(id) ON DELETE CASCADE ON UPDATE CASCADE,
    PRIMARY KEY (id, date_time)
) STRICT;
