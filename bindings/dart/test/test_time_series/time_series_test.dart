import "package:path/path.dart" show join;
import 'package:test/test.dart';
import 'dart:io';
import 'package:psr_database_sqlite/psr_database_sqlite.dart' as psr;

void main() {
  group('Test Time Series Simple', () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_time_series");
    final databasePath = join(dirPath, "test_read_time_series.sqlite");
    final schemaPath = join(dirPath, "test_read_time_series.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() => null);

    test("CRUD", () {
      db.createElement("Configuration", {"label": "Toy Case", "value1": 1.0});

      db.createElement("Resource", {
        "label": "Resource 1",
        "group1": {
          "date_time": [DateTime(2020), DateTime(2021)],
          "some_vector1": [1.0, 2.0],
          "some_vector2": [3.0, null],
        }
      });

      db.createElement("Resource", {
        "label": "Resource 2",
        "group1": {
          "date_time": [DateTime(2000), DateTime(2001), DateTime(2005)],
          "some_vector1": [1.0, null, 3.0],
        }
      });

      db.createElement("Resource", {
        "label": "Resource 3",
      });

      final table_1 = db.readTimeSeriesTable("Resource", "some_vector1", "Resource 1");
      expect(table_1.length, 2);
      expect(table_1[0]["some_vector1"], 1.0);
      expect(table_1[1]["some_vector1"], 2.0);
      expect(table_1[0]["date_time"], DateTime(2020).toIso8601String());
      expect(table_1[1]["date_time"], DateTime(2021).toIso8601String());

      final table_2 = db.readTimeSeriesTable("Resource", "some_vector2", "Resource 1");
      expect(table_2.length, 2);
      expect(table_2[0]["some_vector2"], 3.0);
      expect(table_2[1]["some_vector2"], null);
      expect(table_2[0]["date_time"], DateTime(2020).toIso8601String());

      final table_3 = db.readTimeSeriesTable("Resource", "some_vector1", "Resource 2");
      expect(table_3.length, 3);
      expect(table_3[0]["some_vector1"], 1.0);
      expect(table_3[1]["some_vector1"], null);
      expect(table_3[2]["some_vector1"], 3.0);
      expect(table_3[0]["date_time"], DateTime(2000).toIso8601String());
      expect(table_3[1]["date_time"], DateTime(2001).toIso8601String());
      expect(table_3[2]["date_time"], DateTime(2005).toIso8601String());

      final table_4 = db.readTimeSeriesTable("Resource", "some_vector2", "Resource 2");
      expect(table_4.length, 3);
      expect(table_4[0]["some_vector2"], null);
      expect(table_4[1]["some_vector2"], null);
      expect(table_4[2]["some_vector2"], null);
      expect(table_4[0]["date_time"], DateTime(2000).toIso8601String());
      expect(table_4[1]["date_time"], DateTime(2001).toIso8601String());
      expect(table_4[2]["date_time"], DateTime(2005).toIso8601String());

      final table_5 = db.readTimeSeriesTable("Resource", "some_vector1", "Resource 3");
      expect(table_5.length, 0);

      db.updateTimeSeries("Resource", "some_vector1", "Resource 1", 10.0, {"date_time": DateTime(2021)});
      final table_6 = db.readTimeSeriesTable("Resource", "some_vector1", "Resource 1");
      expect(table_6[1]["some_vector1"], 10.0);

      db.updateTimeSeries("Resource", "some_vector2", "Resource 1", 20.0, {"date_time": DateTime(2021)});
      final table_7 = db.readTimeSeriesTable("Resource", "some_vector2", "Resource 1");
      expect(table_7[1]["some_vector2"], 20.0);

      db.deleteTimeSeries("Resource", "group1", "Resource 2");
      final table_8 = db.readTimeSeriesTable("Resource", "some_vector1", "Resource 2");
      expect(table_8.length, 0);
      final table_9 = db.readTimeSeriesTable("Resource", "some_vector2", "Resource 2");
      expect(table_9.length, 0);
    });

    tearDown(() {
      db.close();
      final file = File(databasePath);
      if (file.existsSync()) {
        file.deleteSync();
      }
    });
  });

  group('Test Time Series two dimensions', () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_time_series");
    final databasePath = join(dirPath, "test_time_series_two_dimensions.sqlite");
    final schemaPath = join(dirPath, "test_read_time_series.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() => null);

    test("CRUD", () {
      db.createElement("Configuration", {"label": "Toy Case", "value1": 1.0});

      db.createElement("Resource", {
        "label": "Resource 1",
        "group1": {
          "date_time": [DateTime(2020), DateTime(2021)],
          "some_vector1": [1.0, 2.0],
          "some_vector2": [3.0, null],
        },
        "group2": {
          "date_time": [DateTime(2020), DateTime(2021), DateTime(2021)],
          "block": [1, 1, 2],
          "some_vector3": [1.0, 2.0, 4.0],
          "some_vector4": [3.0, null, 5.0],
        }
      });

      final table_1 = db.readTimeSeriesTable("Resource", "some_vector1", "Resource 1");
      expect(table_1.length, 2);
      expect(table_1[0]["some_vector1"], 1.0);
      expect(table_1[1]["some_vector1"], 2.0);
      expect(table_1[0]["date_time"], DateTime(2020).toIso8601String());
      expect(table_1[1]["date_time"], DateTime(2021).toIso8601String());

      final table_2 = db.readTimeSeriesTable("Resource", "some_vector2", "Resource 1");
      expect(table_2.length, 2);
      expect(table_2[0]["some_vector2"], 3.0);
      expect(table_2[1]["some_vector2"], null);
      expect(table_2[0]["date_time"], DateTime(2020).toIso8601String());

      final table_3 = db.readTimeSeriesTable("Resource", "some_vector3", "Resource 1");
      expect(table_3.length, 3);
      expect(table_3[0]["some_vector3"], 1.0);
      expect(table_3[1]["some_vector3"], 2.0);
      expect(table_3[2]["some_vector3"], 4.0);
      expect(table_3[0]["date_time"], DateTime(2020).toIso8601String());
      expect(table_3[1]["date_time"], DateTime(2021).toIso8601String());
      expect(table_3[2]["date_time"], DateTime(2021).toIso8601String());
      expect(table_3[0]["block"], 1);
      expect(table_3[1]["block"], 1);
      expect(table_3[2]["block"], 2);

      db.deleteTimeSeries("Resource", "group2", "Resource 1");
      final table_4 = db.readTimeSeriesTable("Resource", "some_vector3", "Resource 1");
      expect(table_4.length, 0);

      final table_5 = db.readTimeSeriesTable("Resource", "some_vector1", "Resource 1");
      expect(table_5.length, 2);
      expect(table_5[0]["some_vector1"], 1.0);
      expect(table_5[1]["some_vector1"], 2.0);
      expect(table_5[0]["date_time"], DateTime(2020).toIso8601String());
      expect(table_5[1]["date_time"], DateTime(2021).toIso8601String());
    });

    tearDown(() {
      db.close();
      final file = File(databasePath);
      if (file.existsSync()) {
        file.deleteSync();
      }
    });
  });

  group('Test Time Series errors', () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_time_series");
    final databasePath = join(dirPath, "test_time_series_errors.sqlite");
    final schemaPath = join(dirPath, "test_read_time_series.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() => null);

    test("CRUD", () {
      db.createElement("Configuration", {"label": "Toy Case", "value1": 1.0});

      expect(
          () => db.createElement("Resource", {
                "label": "Resource 1",
                "group1": {
                  "date_time": [DateTime(2020), DateTime(2021)],
                  "some_vector3": [1.0, 2.0],
                  "some_vector2": [3.0, null],
                },
                "group2": {
                  "date_time": [DateTime(2020), DateTime(2021), DateTime(2021)],
                  "block": [1, 1, 2],
                  "some_vector3": [1.0, 2.0, 4.0],
                  "some_vector4": [3.0, null, 5.0],
                }
              }),
          throwsException);

      expect(
          () => db.createElement("Resource", {
                "label": "Resource 1",
                "group1": {
                  "date_time": [DateTime(2020), DateTime(2021)],
                  "some_vector30": [1.0, 2.0],
                  "some_vector2": [3.0, null],
                },
                "group2": {
                  "date_time": [DateTime(2020), DateTime(2021), DateTime(2021)],
                  "block": [1, 1, 2],
                  "some_vector3": [1.0, 2.0, 4.0],
                  "some_vector4": [3.0, null, 5.0],
                }
              }),
          throwsException);

      expect(
          () => db.createElement("Resource", {
                "label": "Resource 1",
                "group1": {
                  "date_time": [DateTime(2020), DateTime(2021)],
                  "block": [1, 1],
                  "some_vector1": [1.0, 2.0],
                  "some_vector2": [3.0, null],
                },
                "group2": {
                  "date_time": [DateTime(2020), DateTime(2021), DateTime(2021)],
                  "block": [1, 1, 2],
                  "some_vector3": [1.0, 2.0, 4.0],
                  "some_vector4": [3.0, null, 5.0],
                }
              }),
          throwsException);

      db.createElement("Resource", {
        "label": "Resource 1",
        "group1": {
          "date_time": [DateTime(2020), DateTime(2021)],
          "some_vector1": [1.0, 2.0],
          "some_vector2": [3.0, null],
        },
        "group2": {
          "date_time": [DateTime(2020), DateTime(2021), DateTime(2021)],
          "block": [1, 1, 2],
          "some_vector3": [1.0, 2.0, 4.0],
          "some_vector4": [3.0, null, 5.0],
        }
      });

      expect(() => db.createElement("Resource", {"label": "Resource 1", "group1": {}}), throwsException);

      expect(() => db.updateTimeSeries("Resource", "some_vector1", "Resource 1", 10.0, {"date_time": DateTime(2055)}),
          throwsException);
    });

    tearDown(() {
      db.close();
      final file = File(databasePath);
      if (file.existsSync()) {
        file.deleteSync();
      }
    });
  });

  group('Test Time Series PSRHub Table', () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_time_series");
    final databasePath = join(dirPath, "test_time_series_psrhub.sqlite");
    final schemaPath = join(dirPath, "test_read_time_series.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() => null);

    test("CRUD", () {
      db.createElement("Configuration", {"label": "Toy Case", "value1": 1.0});

      db.createElement("Resource", {
        "label": "Resource 1",
        "group1": {
          "date_time": [DateTime(2020), DateTime(2021)],
          "some_vector1": [1.0, 2.0],
          "some_vector2": [3.0, null],
        },
        "group2": {
          "date_time": [DateTime(2020), DateTime(2021), DateTime(2021)],
          "block": [1, 1, 2],
          "some_vector3": [1.0, 2.0, 4.0],
          "some_vector4": [3.0, null, 5.0],
        }
      });

      final dateTimeAnswer = <String>[
        DateTime(2020).toIso8601String(),
        DateTime(2021).toIso8601String(),
        DateTime(2021).toIso8601String()
      ];
      final someVector3Answer = [1.0, 2.0, 4.0];
      final someVector4Answer = [3.0, null, 5.0];
      final blockAnswer = <int>[1, 1, 2];

      final tableGroup =
          db.readElementGroupOfTimeSeriesAttributesFromId("Resource", 1, "group2", <String>["date_time", "block"]);

      for (var i = 0; i < tableGroup.length; i++) {
        expect(tableGroup[i]["date_time"], dateTimeAnswer[i]);
        expect(tableGroup[i]["block"], blockAnswer[i]);
        expect(tableGroup[i]["some_vector3"], someVector3Answer[i]);
        expect(tableGroup[i]["some_vector4"], someVector4Answer[i]);
      }

      db.updateElementGroupOfTimeSeriesFromId("Resource", 1, "group2", {
        "some_vector3": [10.0, 11.0],
        "date_time": [DateTime(2022), DateTime(2023)],
        "block": [3, 4]
      });

      try {
        db.updateElementGroupOfTimeSeriesFromId("Resource", 1, "group2", {
          "some_vector3": [10.0, 11.0, 12.3],
          "date_time": [DateTime(2022), DateTime(2023), DateTime(2023)],
          "block": [3, 4, 4]
        });
      } catch (e) {
        expect(
            e.toString().contains(
                "One of the provided dimension entries already exists in the database. Please provide a new entry or update the existing one."),
            true);
      }
    });

    tearDown(() {
      db.close();
      final file = File(databasePath);
      if (file.existsSync()) {
        file.deleteSync();
      }
    });
  });
}
