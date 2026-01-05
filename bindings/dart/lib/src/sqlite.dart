import 'dart:io';
import 'package:path/path.dart' as p;
import 'database.dart' as db;

class SQLite {
  static String? _customLibraryPath;

  /// Set custom path for the psr_database_c library.
  /// The sqlite3.dll argument is ignored since SQLite is compiled into psr_database_c.
  /// This method infers the psr_database_c library path from the sqlite3.dll path.
  static void openCustomDLL(String path) {
    // The test passes sqlite3.dll path, but we need libpsr_database_c.dll
    // Replace sqlite3.dll with libpsr_database_c.dll in the same directory
    final dir = p.dirname(path);
    if (Platform.isWindows) {
      _customLibraryPath = p.join(dir, 'libpsr_database_c.dll');
    } else if (Platform.isLinux) {
      _customLibraryPath = p.join(dir, 'libpsr_database_c.so');
    } else if (Platform.isMacOS) {
      _customLibraryPath = p.join(dir, 'libpsr_database_c.dylib');
    }
    if (_customLibraryPath != null) {
      db.setLibraryPath(_customLibraryPath!);
    }
  }

  static String? get customLibraryPath => _customLibraryPath;
}
