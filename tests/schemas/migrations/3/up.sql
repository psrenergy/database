-- add_test_3
PRAGMA user_version = 3;

CREATE TABLE Test3 (
    id INTEGER PRIMARY KEY,
    label TEXT UNIQUE NOT NULL,
    capacity INTEGER,
    some_other_coefficient REAL
) STRICT;