import "package:path/path.dart" show join;
import 'package:test/test.dart';
import 'dart:io';
import 'package:psr_database_sqlite/psr_database_sqlite.dart' as psr;

void main() {
  group('Create parameters in database', () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_create");
    final databasePath = join(dirPath, "test_create_parameters.sqlite");
    final schemaPath = join(dirPath, "test_create_parameters.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() => null);

    test("Create parameters", () {
      expect(() => db.createElement("Configuration", {"label": "Toy Case", "value1": "wrong"}), throwsException);
      db.createElement("Configuration", {"label": "Toy Case", "value1": 1.0, "date_time_value2": null});
      db.createElement("Resource", {"label": "Resource 2"});
      db.createElement("Resource", {"label": "Resource 1", "type": "E"});
      expect(() => db.createElement("Resource", {"label": "Resource 4", "type3": "E"}), throwsException);
    });

    tearDown(() {
      db.close();
      final file = File(databasePath);
      if (file.existsSync()) {
        file.deleteSync();
      }
    });
  });

  group("Test create scalar parameter date", () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_create");
    final databasePath = join(dirPath, "test_create_scalar_parameter_date.sqlite");
    final schemaPath = join(dirPath, "test_create_scalar_parameter_date.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() => null);

    test("Create scalar parameter date", () {
      db.createElement("Configuration",
          {"label": "Toy Case", "date_initial": DateTime(2000), "date_final": DateTime(2001, 10, 12, 23, 45, 12)});
      // Creating a duplicate label should throw due to UNIQUE constraint
      expect(
          () => db.createElement(
              "Configuration", {"label": "Toy Case", "date_initial": DateTime(2000), "date_final": "2001-10-12 23:45:12"}),
          throwsException);
      // Note: Date format validation is not performed at the Dart layer in thin wrapper approach
      // SQLite TEXT columns accept any string value
      db.createElement("Resource", {"label": "Resource 1", "date_initial_1": "2000-01"});
      db.createElement("Resource", {"label": "Resource 2", "date_initial_1": "20001334"});
    });

    tearDown(() {
      db.close();
      final file = File(databasePath);
      if (file.existsSync()) {
        file.deleteSync();
      }
    });
  });

  group("Test create parameters and vectors", () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_create");
    final databasePath = join(dirPath, "test_create_parameters_and_vectors.sqlite");
    final schemaPath = join(dirPath, "test_create_parameters_and_vectors.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() {
      db.createElement("Configuration", {"label": "Toy Case", "value1": 1.0});
      db.createElement("Resource", {
        "label": "Resource 1",
        "type": "E",
        "some_value": [1.0, 2.0, 3.0]
      });
      db.createElement("Resource", {"label": "name"});
      db.createElement("Cost", {"label": "Cost 1", "value": 30.0});
      db.createElement("Cost", {"label": "Cost 2", "value": 20.0});
      db.createElement("Plant", {
        "label": "Plant 1",
        "capacity": 50.0,
        "some_factor": [0.1, 0.3],
        "cost_id": ["Cost 1", "Cost 2"]
      });
      db.createElement("Plant", {
        "label": "Plant 2",
        "capacity": 50.0,
        "some_factor": [0.1, 0.3, 0.5],
        "cost_id": ["Cost 1", "Cost 2", "Cost 1"]
      });
    });

    test("Create parameters and vectors", () {
      expect(() => db.createElement("Plant", {"label": "Plant 3", "generation": "some_file.txt"}), throwsException);
      expect(() => db.createElement("Plant", {"label": "Plant 2", "capacity": 50.0, "some_factor": []}), throwsException);
      db.createElement("Plant", {"label": "Plant 3", "resource_id": 1});
      expect(() => db.createElement("Resource", {"label": "Resource 1", "type": "E", "some_value": 1.0}), throwsException);
      db.createElement("Plant", {"label": "Plant 4", "resource_id": null});
    });

    tearDown(() {
      db.close();
      final file = File(databasePath);
      if (file.existsSync()) {
        file.deleteSync();
      }
    });
  });

  group("Test create sets with relations", () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_create");
    final databasePath = join(dirPath, "test_create_sets_with_relations.sqlite");
    final schemaPath = join(dirPath, "test_create_sets_with_relations.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() {
      db.createElement("Configuration", {"label": "Toy Case", "some_value": 1.0});
      db.createElement("Product", {"label": "Sugar", "unit": "Kg"});
      db.createElement("Product", {"label": "Sugarcane", "unit": "ton"});
      db.createElement("Product", {"label": "Molasse", "unit": "ton"});
      db.createElement("Product", {"label": "Bagasse", "unit": "ton"});
      expect(() => db.createElement("Product", {"label": "Bagasse 2", "unit": 30}), throwsException);
    });

    test("Create sets with relations", () {
      db.createElement("Process", {
        "label": "Sugar Mill",
        "product_set": ["Sugarcane"],
        "factor": [1.0]
      });
      expect(() => db.createElement("Process", {
            "label": "Sugar Mill 2",
            "product_set": ["Sugar"],
            "factor": ["wrong"]
          }), throwsException);
      expect(() => db.createElement("Process", {
            "label": "Sugar Mill 3",
            "product_set": ["Some Sugar"],
            "factor": [1.0]
          }), throwsException);
      expect(() => db.createElement("Process", {
            "label": "Sugar Mill 3",
            "product_set": ["Sugear"],
            "factor": []
          }), throwsException);
    });

    tearDown(() {
      db.close();
      final file = File(databasePath);
      if (file.existsSync()) {
        file.deleteSync();
      }
    });
  });

  group("Test create vectors with different sizes in same group", () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_create");
    final databasePath = join(dirPath, "test_create_vectors_with_different_sizes_in_same_group.sqlite");
    final schemaPath = join(dirPath, "test_create_vectors_with_different_sizes_in_same_group.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() {
      db.createElement("Configuration", {"label": "Toy Case", "value1": 1.0});
    });

    test("Create vectors with different sizes in same group", () {
      expect(
          () => db.createElement("Resource", {
                "label": "Resource 1",
                "type": "E",
                "some_vector1": [1.0],
                "some_vector2": [1.0, 2.0]
              }),
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

  group("Test create vectors with relations", () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_create");
    final databasePath = join(dirPath, "test_create_vectors_with_relations.sqlite");
    final schemaPath = join(dirPath, "test_create_vectors_with_relations.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() {
      db.createElement("Configuration", {"label": "Toy Case", "some_value": 1.0});
      db.createElement("Product", {"label": "Sugar", "unit": "Kg"});
      db.createElement("Product", {"label": "Sugarcane", "unit": "ton"});
      db.createElement("Product", {"label": "Molasse", "unit": "ton"});
      db.createElement("Product", {"label": "Bagasse", "unit": "ton"});
      expect(() => db.createElement("Product", {"label": "Bagasse 2", "unit": 30}), throwsException);
    });

    test("Create vectors with relations", () {
      db.createElement(
        "Process",
        {
          "label": "Sugar Mill",
          "product_input": ["Sugarcane"],
          "factor_input": [1.0],
          "product_output": ["Sugar", "Molasse", "Bagasse"],
          "factor_output": [0.3, 0.3, 0.4],
        },
      );
      expect(
          () => db.createElement(
                "Process",
                {
                  "label": "Sugar Mill 2",
                  "product_input": ["Sugar"],
                  "factor_input": ["wrong"],
                  "product_output": ["Sugarcane"],
                  "factor_output": [1.0],
                },
              ),
          throwsException);
      expect(
          () => db.createElement(
                "Process",
                {
                  "label": "Sugar Mill 3",
                  "product_input": ["Some Sugar"],
                  "factor_input": [1.0],
                  "product_output": ["Sugarcane"],
                  "factor_output": [1.0],
                },
              ),
          throwsException);
      expect(
          () => db.createElement(
                "Process",
                {
                  "label": "Sugar Mill 3",
                  "product_input": ["Some Sugar"],
                  "factor_input": [],
                  "product_output": ["Sugarcane"],
                  "factor_output": [1.0],
                },
              ),
          throwsException);

      db.createElement(
        "Process",
        {
          "label": "Sugar Mill 2",
          "product_input": ["Sugar"],
          "factor_input": [1.0],
        },
      );
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
