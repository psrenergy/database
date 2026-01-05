import "package:path/path.dart" show join;
import 'package:test/test.dart';
import 'dart:io';
import 'package:psr_database_sqlite/psr_database_sqlite.dart' as psr;

void main() {
  group('Delete elements in database', () {
    final appPath = Directory.current.path;
    final dirPath = join(appPath, "test", "test_delete");
    final databasePath = join(dirPath, "test_delete_element.sqlite");
    final schemaPath = join(dirPath, "test_delete_element.sql");

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
      db.createElement("Resource", {"label": "Resource 1"});
    });

    test("Delete parameters", () {
      db.deleteElementFromLabel("Resource", "Resource 1");
      db.deleteElementFromId("Resource", 1);
      // TODO Add test to see that there are no more resources
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
