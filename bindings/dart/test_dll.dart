import 'dart:ffi';
import 'dart:io';

void main() {
  print('Testing DLL load from assets directory...');

  // Change to assets directory so dependent DLLs can be found
  final originalDir = Directory.current;
  Directory.current = Directory('C:/Development/Personal/psr_database/bindings/dart/assets');

  try {
    final lib = DynamicLibrary.open('C:/Development/Personal/psr_database/bindings/dart/assets/libpsr_database_c.dll');
    print('Success: $lib');
  } catch (e) {
    print('Error: $e');
  } finally {
    Directory.current = originalDir;
  }
}
