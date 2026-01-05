PRAGMA user_version = 1;
PRAGMA foreign_keys = ON;

CREATE TABLE Configuration (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    label TEXT UNIQUE NOT NULL,
    value1 REAL NOT NULL DEFAULT 100,
    enum1 TEXT NOT NULL DEFAULT 'A' CHECK(enum1 IN ('A', 'B', 'C'))
) STRICT;

CREATE TABLE Product (
    id INTEGER PRIMARY KEY,
    label TEXT UNIQUE NOT NULL
) STRICT;

CREATE TABLE Resource (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    label TEXT UNIQUE NOT NULL,
    type TEXT NOT NULL DEFAULT "D" CHECK(type IN ('D', 'E', 'F'))
) STRICT;

CREATE TABLE Resource_vector_some_group (
    id INTEGER, 
    vector_index INTEGER NOT NULL,
    some_value_1 REAL NOT NULL,
    some_value_2 REAL,
    product_id INTEGER,
    FOREIGN KEY(id) REFERENCES Resource(id) ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY(product_id) REFERENCES Product(id) ON DELETE SET NULL ON UPDATE CASCADE,
    PRIMARY KEY (id, vector_index)
) STRICT;