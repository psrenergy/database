import "package:path/path.dart" show join;
import 'package:test/test.dart';
import 'dart:io';
import 'package:psr_database_sqlite/psr_database_sqlite.dart' as psr;

void main() {
  group('Update scalar parameters in database', () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_update");
    final databasePath = join(dirPath, "test_update_scalar_parameters.sqlite");
    final schemaPath = join(dirPath, "test_update_scalar_parameters.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() {
      db.createElement("Configuration", {"label": "Toy Case"});
      db.createElement("Resource", {"label": "Resource 2"});
      db.createElement("Resource", {"label": "Resource 1", "type": "E"});
      db.createElement("Cost", {"label": "Cost 1"});
      db.createElement("Cost", {"label": "Cost 2", "value1": 10.0});
      db.createElement("Calendar", {"label": "Calendar 1", "date_initial": DateTime(2020, 1, 1)});
    });

    test("Update scalar parameters", () {
      db.updateElementScalarAttributesFromLabel("Cost", "Cost 1", {"value1": 20.0});

      var element = db.readElementScalarAttributesFromLabel("Cost", "Cost 1");
      expect(element["label"], "Cost 1");
      expect(element["value1"], 20.0);

      db.updateElementScalarAttributesFromLabel("Cost", "Cost 1", {"value1": 30.0});
      element = db.readElementScalarAttributesFromLabel("Cost", "Cost 1");
      expect(element["value1"], 30.0);

      db.updateElementScalarAttributesFromLabel("Resource", "Resource 1", {"type": "D"});
      element = db.readElementScalarAttributesFromLabel("Resource", "Resource 1");
      expect(element["type"], "D");

      db.updateElementScalarAttributesFromLabel("Resource", "Resource 1", {"some_value_1": 1.0});
      element = db.readElementScalarAttributesFromLabel("Resource", "Resource 1");
      expect(element["some_value_1"], 1.0);

      db.updateElementScalarAttributesFromLabel("Resource", "Resource 1", {"some_value_1": 2.0, "some_value_2": 3.0});
      element = db.readElementScalarAttributesFromLabel("Resource", "Resource 1");
      expect(element["some_value_1"], 2.0);
      expect(element["some_value_2"], 3.0);

      db.updateElementScalarAttributesFromLabel("Resource", "Resource 1", {"some_value_2": null});
      element = db.readElementScalarAttributesFromLabel("Resource", "Resource 1");
      expect(element["some_value_2"], null);

      db.updateElementScalarAttributesFromLabel("Calendar", "Calendar 1", {"date_initial": DateTime(2020, 1, 2)});
      element = db.readElementScalarAttributesFromLabel("Calendar", "Calendar 1");
      expect(element["date_initial"], DateTime(2020, 1, 2));
    });

    tearDown(() {
      db.close();
      final file = File(databasePath);
      if (file.existsSync()) {
        file.deleteSync();
      }
    });
  });

  group("Update vector parameters in database", () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_update");
    final databasePath = join(dirPath, "test_update_vector_parameters.sqlite");
    final schemaPath = join(dirPath, "test_update_vector_parameters.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );
    setUp(() {
      db.createElement("Configuration", {
        "label": "Toy Case",
        "value1": 1.0,
      });
      db.createElement("Product", {
        "label": "Product 1",
      });
      db.createElement("Product", {
        "label": "Product 2",
      });
      db.createElement("Resource", {
        "label": "Resource 1",
        "type": "E",
        "some_value_1": [1.0, 2.0, 3.0],
        "some_value_2": [4.0, 5.0, 6.0],
        "product_id": [1, 2, 1],
      });
      db.createElement("Resource", {
        "label": "Resource 2",
        "type": "E",
      });
    });

    test("Update vector parameters", () {
      var element = db.readElementGroupOfVectorAttributesFromLabel("Resource", "Resource 1", "some_group");
      expect(element[0]["some_value_1"], 1.0);
      expect(element[1]["some_value_1"], 2.0);
      expect(element[2]["some_value_1"], 3.0);
      expect(element[0]["product_id"], 1);
      expect(element[1]["product_id"], 2);
      expect(element[2]["product_id"], 1);

      db.updateElementGroupOfVectorAttributesFromLabel("Resource", "Resource 1", "some_group", {
        "some_value_1": [4.0, 5.0, 6.0],
        "product_id": [2, 1, 2],
      });
      element = db.readElementGroupOfVectorAttributesFromLabel("Resource", "Resource 1", "some_group");
      expect(element[0]["some_value_1"], 4.0);
      expect(element[1]["some_value_1"], 5.0);
      expect(element[2]["some_value_1"], 6.0);
      expect(element[0]["product_id"], 2);
      expect(element[1]["product_id"], 1);
      expect(element[2]["product_id"], 2);

      db.updateElementGroupOfVectorAttributesFromLabel("Resource", "Resource 1", "some_group", {
        "some_value_1": [4.0, 5.0, 6.0],
        "some_value_2": [7.0, 8.0, null]
      });
      element = db.readElementGroupOfVectorAttributesFromLabel("Resource", "Resource 1", "some_group");
      expect(element[0]["some_value_1"], 4.0);
      expect(element[1]["some_value_1"], 5.0);
      expect(element[2]["some_value_1"], 6.0);
      expect(element[0]["some_value_2"], 7.0);
      expect(element[1]["some_value_2"], 8.0);
      expect(element[2]["some_value_2"], null);

      final newValues = {
        "some_value_1": <dynamic>[3.0, 2.0, 1.0],
        "some_value_2": <dynamic>[6.0, 4.0, 2.0],
        "product_id": <dynamic>[1, 1, 1],
      };
      db.updateElementGroupOfVectorAttributesFromLabel("Resource", "Resource 1", "some_group", newValues);
    });

     tearDown(() {
      db.close();
      final file = File(databasePath);
      if (file.existsSync()) {
        file.deleteSync();
      }
    });
  });

  group("Update set parameters", () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_update");
    final databasePath = join(dirPath, "test_update_set_parameters.sqlite");
    final schemaPath = join(dirPath, "test_update_set_parameters.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() {
      db.createElement("Resource", {
        "label": "Resource 1",
        "type": "E",
        "some_value_1": [1.0, 2.0, 3.0],
        "some_value_2": [4.0, 5.0, 6.0],
      });
    });

    test("Update set parameters", () {
      var element = db.readElementGroupOfSetAttributesFromLabel("Resource", "Resource 1", "some_group");
      expect(element[0]["some_value_1"], 1.0);
      expect(element[1]["some_value_1"], 2.0);
      expect(element[2]["some_value_1"], 3.0);
      expect(element[0]["some_value_2"], 4.0);
      expect(element[1]["some_value_2"], 5.0);
      expect(element[2]["some_value_2"], 6.0);

      db.updateElementGroupOfSetAttributesFromLabel("Resource", "Resource 1", "some_group", {
        "some_value_1": [4.0, 5.0, 6.0]
      });

      element = db.readElementGroupOfSetAttributesFromLabel("Resource", "Resource 1", "some_group");
      expect(element[0]["some_value_1"], 4.0);
      expect(element[1]["some_value_1"], 5.0);
      expect(element[2]["some_value_1"], 6.0);
      expect(element[0]["some_value_2"], null);
      expect(element[1]["some_value_2"], null);
      expect(element[2]["some_value_2"], null);

      db.updateElementGroupOfSetAttributesFromLabel("Resource", "Resource 1", "some_group", {
        "some_value_1": [4.0, 5.0, 6.0],
        "some_value_2": [7.0, 8.0, 9.0]
      });
      element = db.readElementGroupOfSetAttributesFromLabel("Resource", "Resource 1", "some_group");
      expect(element[0]["some_value_1"], 4.0);
      expect(element[1]["some_value_1"], 5.0);
      expect(element[2]["some_value_1"], 6.0);
      expect(element[0]["some_value_2"], 7.0);
      expect(element[1]["some_value_2"], 8.0);
      expect(element[2]["some_value_2"], 9.0);

      expect(
        () => db.updateElementGroupOfSetAttributesFromLabel("Resource", "Resource 1", "some_group", {
          "some_value_3": [1.0, 2.0, 3.0]
        }),
        throwsException,
      );

      expect(
        () => db.updateElementGroupOfSetAttributesFromLabel("Resource", "Resource 1", "some_group", {
          "some_value_1": [1, 2, 3]
        }),
        throwsException,
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