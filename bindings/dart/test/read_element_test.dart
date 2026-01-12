import 'package:psr_database/psr_database.dart';
import 'package:test/test.dart';
import 'package:path/path.dart' as path;

void main() {
  final testsPath = path.join(
    path.current,
    '..',
    '..',
    'tests',
  );

  group('getElementIds', () {
    test('returns empty list for empty collection', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        final ids = db.getElementIds('Resource');
        expect(ids, isEmpty);
      } finally {
        db.close();
      }
    });

    test('returns all element IDs in order', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Resource', {'label': 'Resource 1'});
        db.createElement('Resource', {'label': 'Resource 2'});
        db.createElement('Resource', {'label': 'Resource 3'});

        final ids = db.getElementIds('Resource');
        expect(ids, hasLength(3));
        expect(ids[0], equals(1));
        expect(ids[1], equals(2));
        expect(ids[2], equals(3));
      } finally {
        db.close();
      }
    });

    test('returns IDs only for specified collection', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Resource', {'label': 'Resource 1'});
        db.createElement('Cost', {'label': 'Cost 1', 'value': 10.0});

        final resourceIds = db.getElementIds('Resource');
        final costIds = db.getElementIds('Cost');

        expect(resourceIds, hasLength(1));
        expect(costIds, hasLength(1));
      } finally {
        db.close();
      }
    });
  });

  group('readElementScalarAttributes', () {
    test('reads all scalar attributes for element', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1', 'value1': 50.0});

        final attrs = db.readElementScalarAttributes('Configuration', 1);

        expect(attrs['label'], equals('Config 1'));
        expect(attrs['value1'], equals(50.0));
        expect(attrs['enum1'], equals('A')); // default value
        expect(attrs.containsKey('date_initial'), isTrue);
      } finally {
        db.close();
      }
    });

    test('reads scalar attributes with null values', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Cost', {'label': 'Cost 1', 'value': 25.0});

        final attrs = db.readElementScalarAttributes('Cost', 1);

        expect(attrs['label'], equals('Cost 1'));
        expect(attrs['value'], equals(25.0));
        expect(attrs['value_without_default'], isNull);
      } finally {
        db.close();
      }
    });

    test('reads attributes for specific element by ID', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Resource', {'label': 'Resource 1', 'type': 'D'});
        db.createElement('Resource', {'label': 'Resource 2', 'type': 'E'});
        db.createElement('Resource', {'label': 'Resource 3', 'type': 'F'});

        final attrs1 = db.readElementScalarAttributes('Resource', 1);
        final attrs2 = db.readElementScalarAttributes('Resource', 2);
        final attrs3 = db.readElementScalarAttributes('Resource', 3);

        expect(attrs1['label'], equals('Resource 1'));
        expect(attrs1['type'], equals('D'));
        expect(attrs2['label'], equals('Resource 2'));
        expect(attrs2['type'], equals('E'));
        expect(attrs3['label'], equals('Resource 3'));
        expect(attrs3['type'], equals('F'));
      } finally {
        db.close();
      }
    });

    test('throws for non-existent element', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});

        expect(
          () => db.readElementScalarAttributes('Resource', 999),
          throwsA(isA<DatabaseException>()),
        );
      } finally {
        db.close();
      }
    });
  });

  group('readElementVectorGroup', () {
    test('reads vector group attributes for element', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Resource', {
          'label': 'Resource 1',
          'some_value': [1.0, 2.0, 3.0],
        });

        final attrs = db.readElementVectorGroup('Resource', 1, 'some_group');

        expect(attrs['some_value'], equals([1.0, 2.0, 3.0]));
      } finally {
        db.close();
      }
    });

    test('returns empty lists for element without vector data', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Resource', {'label': 'Resource 1'});

        final attrs = db.readElementVectorGroup('Resource', 1, 'some_group');

        expect(attrs['some_value'], isEmpty);
      } finally {
        db.close();
      }
    });

    test('reads multiple attributes from vector group', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Cost', {'label': 'Cost 1', 'value': 10.0});
        db.createElement('Plant', {
          'label': 'Plant 1',
          'some_factor': [0.5, 0.6, 0.7],
          'cost_id': [1, 1, 1],
        });

        final attrs = db.readElementVectorGroup('Plant', 1, 'cost_relation');

        expect(attrs['some_factor'], equals([0.5, 0.6, 0.7]));
        expect(attrs['cost_id'], equals([1, 1, 1]));
        expect(attrs.containsKey('date_some_date'), isTrue);
      } finally {
        db.close();
      }
    });

    test('reads vector data for specific element', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Resource', {
          'label': 'Resource 1',
          'some_value': [1.0, 2.0],
        });
        db.createElement('Resource', {
          'label': 'Resource 2',
          'some_value': [3.0, 4.0, 5.0],
        });

        final attrs1 = db.readElementVectorGroup('Resource', 1, 'some_group');
        final attrs2 = db.readElementVectorGroup('Resource', 2, 'some_group');

        expect(attrs1['some_value'], equals([1.0, 2.0]));
        expect(attrs2['some_value'], equals([3.0, 4.0, 5.0]));
      } finally {
        db.close();
      }
    });
  });

  group('readElementSetGroup', () {
    test('reads set group attributes for element', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Resource', {
          'label': 'Resource 1',
          'some_other_value': [10.0, 20.0, 30.0],
        });

        final attrs = db.readElementSetGroup('Resource', 1, 'some_other_group');

        expect(attrs['some_other_value'], containsAll([10.0, 20.0, 30.0]));
      } finally {
        db.close();
      }
    });

    test('returns empty lists for element without set data', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Resource', {'label': 'Resource 1'});

        final attrs = db.readElementSetGroup('Resource', 1, 'some_other_group');

        expect(attrs['some_other_value'], isEmpty);
      } finally {
        db.close();
      }
    });

    test('reads set data for specific element', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Resource', {
          'label': 'Resource 1',
          'some_other_value': [1.0, 2.0],
        });
        db.createElement('Resource', {
          'label': 'Resource 2',
          'some_other_value': [3.0, 4.0, 5.0],
        });

        final attrs1 = db.readElementSetGroup('Resource', 1, 'some_other_group');
        final attrs2 = db.readElementSetGroup('Resource', 2, 'some_other_group');

        expect(attrs1['some_other_value'], containsAll([1.0, 2.0]));
        expect(attrs2['some_other_value'], containsAll([3.0, 4.0, 5.0]));
      } finally {
        db.close();
      }
    });

    test('reads multiple attributes from set group', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Cost', {'label': 'Cost 1', 'value': 10.0});
        db.createElement('Plant', {
          'label': 'Plant 1',
          'some_other_factor': [0.1, 0.2],
          'cost_rel': [1, 1],
        });

        final attrs = db.readElementSetGroup('Plant', 1, 'other_cost_relation');

        expect(attrs['some_other_factor'], containsAll([0.1, 0.2]));
        expect(attrs['cost_rel'], containsAll([1, 1]));
      } finally {
        db.close();
      }
    });
  });

  group('readElementTimeSeriesGroup', () {
    test('throws for non-existent time series group', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Resource', {'label': 'Resource 1'});

        // Resource doesn't have a time series group in test_read.sql
        expect(
          () => db.readElementTimeSeriesGroup('Resource', 1, 'nonexistent_group', []),
          throwsA(isA<DatabaseException>()),
        );
      } finally {
        db.close();
      }
    });

    test('throws for invalid collection', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});

        expect(
          () => db.readElementTimeSeriesGroup('InvalidCollection', 1, 'group1', []),
          throwsA(isA<DatabaseException>()),
        );
      } finally {
        db.close();
      }
    });
  });

  group('Integration', () {
    test('reads all data types for complete element', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Cost', {'label': 'Cost 1', 'value': 100.0});
        db.createElement('Resource', {
          'label': 'Resource 1',
          'type': 'E',
          'some_value': [1.0, 2.0, 3.0],
          'some_other_value': [10.0, 20.0],
        });

        // Get element ID
        final ids = db.getElementIds('Resource');
        expect(ids, hasLength(1));
        final elementId = ids[0];

        // Read scalars
        final scalars = db.readElementScalarAttributes('Resource', elementId);
        expect(scalars['label'], equals('Resource 1'));
        expect(scalars['type'], equals('E'));

        // Read vectors
        final vectors = db.readElementVectorGroup('Resource', elementId, 'some_group');
        expect(vectors['some_value'], equals([1.0, 2.0, 3.0]));

        // Read sets
        final sets = db.readElementSetGroup('Resource', elementId, 'some_other_group');
        expect(sets['some_other_value'], containsAll([10.0, 20.0]));
      } finally {
        db.close();
      }
    });

    test('handles multiple elements with different data', () {
      final db = Database.fromSchema(
        ':memory:',
        path.join(testsPath, 'test_read', 'test_read.sql'),
      );
      try {
        db.createElement('Configuration', {'label': 'Config 1'});
        db.createElement('Resource', {
          'label': 'Resource A',
          'type': 'D',
          'some_value': [1.0],
        });
        db.createElement('Resource', {
          'label': 'Resource B',
          'type': 'E',
          'some_value': [2.0, 3.0],
        });
        db.createElement('Resource', {
          'label': 'Resource C',
          'type': 'F',
          'some_value': [4.0, 5.0, 6.0],
        });

        final ids = db.getElementIds('Resource');
        expect(ids, hasLength(3));

        for (var i = 0; i < ids.length; i++) {
          final elementId = ids[i];
          final scalars = db.readElementScalarAttributes('Resource', elementId);
          final vectors = db.readElementVectorGroup('Resource', elementId, 'some_group');

          expect(scalars['label'], startsWith('Resource'));
          expect(vectors['some_value'], hasLength(i + 1));
        }
      } finally {
        db.close();
      }
    });
  });
}
