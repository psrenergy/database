import "package:path/path.dart" show join;
import 'package:test/test.dart';
import 'dart:io';
import 'package:psr_database_sqlite/psr_database_sqlite.dart' as psr;

void main() {
  group('Read parameters', () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_read");
    final databasePath = join(dirPath, "test_read.sqlite");
    final schemaPath = join(dirPath, "test_read.sql");

    psr.SQLite.openCustomDLL(join(appPath, "assets", "sqlite3.dll"));
    if (File(databasePath).existsSync()) {
      File(databasePath).deleteSync();
    }
    final db = psr.DatabaseSQLite.fromSchema(
      databasePath: databasePath,
      schemaPath: schemaPath,
    );

    setUp(() {
      db.createElement("Configuration", {"label": "Toy Case", "date_initial": DateTime(2020, 1, 1), "date_final": null});
      db.createElement("Resource", {
        "label": "Resource 1",
        "some_value": [1.0, 2.0, 3.0]
      });
      db.createElement("Resource", {
        "label": "Resource 2",
        "some_value": [1.0, 2.0, 4.0]
      });
      db.createElement("Cost", {"label": "Cost 1"});
      db.createElement("Cost", {"label": "Cost 2", "value": 10.0});
      db.createElement("Plant", {
        "label": "Plant 1",
        "capacity": 2.02,
        "some_factor": [1.0],
        "date_some_date": [DateTime(2020, 1, 1)],
        "cost_id": ["Cost 1"],
        "resource_id": "Resource 1",
      });
      db.createElement("Plant", {
        "label": "Plant 2",
        "capacity": 53.0,
        "some_factor": [1.0, 2.0],
        "date_some_date": [DateTime(2020, 1, 1), DateTime(2020, 1, 2)],
        "cost_id": ["Cost 1", "Cost 2"]
      });
      db.createElement("Plant", {"label": "Plant 3", "capacity": 54.0, "plant_turbine_to": "Plant 2"});
      db.createElement("Plant", {
        "label": "Plant 4",
        "capacity": 53.0,
        "some_factor": [1.0, 2.0],
        "date_some_date": [DateTime(2020, 1, 1), DateTime(2020, 1, 2)],
        "cost_id": ["Cost 2", "Cost 1"]
      });
      db.setTimeSeriesFile("Plant", {"wind_speed": "some_file.txt", "wind_direction": "some_file2"});
    });

    test("Read parameters", () {
      final configurations = db.readElementScalarAttributesFromLabel("Configuration", "Toy Case");
      expect(configurations["label"], "Toy Case");
      expect(configurations["date_initial"], DateTime(2020, 1, 1));
      expect(configurations["date_final"], null);

      final ids = db.getElementsIdsFromCollectionId("Plant");
      expect(ids.length, 4);
      expect(ids[0], 1);
      expect(ids[1], 2);
      expect(ids[2], 3);
      expect(ids[3], 4);

      final plant1ScalarAttributes = db.readElementScalarAttributesFromLabel("Plant", "Plant 1");
      expect(plant1ScalarAttributes["label"], "Plant 1");
      expect(plant1ScalarAttributes["capacity"], 2.02);
      expect(plant1ScalarAttributes["resource_id"], 1);

      final plant1TimeSeries = db.readCollectionTimeSeriesFiles("Plant");
      expect(plant1TimeSeries["wind_speed"], "some_file.txt");
      expect(plant1TimeSeries["wind_direction"], "some_file2");

      final plant2CostRelation = db.readElementGroupOfVectorAttributesFromLabel("Plant", "Plant 2", "cost_relation");
      expect(plant2CostRelation[0]["some_factor"], 1.0);
      expect(plant2CostRelation[1]["some_factor"], 2.0);
      expect(plant2CostRelation[0]["date_some_date"], DateTime(2020, 1, 1));
      expect(plant2CostRelation[1]["date_some_date"], DateTime(2020, 1, 2));
      expect(plant2CostRelation[0]["cost_id"], 1);
      expect(plant2CostRelation[1]["cost_id"], 2);

      final plant3ScalarAttributes = db.readElementScalarAttributesFromLabel("Plant", "Plant 3");
      expect(plant3ScalarAttributes["label"], "Plant 3");
      expect(plant3ScalarAttributes["capacity"], 54.0);
      expect(plant3ScalarAttributes["plant_turbine_to"], 2);

      final plant4CostRelation = db.readElementGroupOfVectorAttributesFromLabel("Plant", "Plant 4", "cost_relation");
      expect(plant4CostRelation[0]["some_factor"], 1.0);
      expect(plant4CostRelation[1]["some_factor"], 2.0);
      expect(plant4CostRelation[0]["date_some_date"], DateTime(2020, 1, 1));
      expect(plant4CostRelation[1]["date_some_date"], DateTime(2020, 1, 2));
      expect(plant4CostRelation[0]["cost_id"], 2);
      expect(plant4CostRelation[1]["cost_id"], 1);
    });

    tearDown(() {
      db.close();
      File file = File(databasePath);
      if (file.existsSync()) {
        file.deleteSync();
      }
    });
  });
}
