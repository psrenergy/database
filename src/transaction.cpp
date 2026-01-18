#include "psr/transaction.h"

#include "psr/database.h"

#include <stdexcept>

namespace psr {

Transaction::Transaction(Database& db) : db_(db) {
    // Note: This requires adding public transaction methods to Database class
    // For now, this is a design document showing how it should work
    // Implementation would call: db_.begin_transaction();
    throw std::runtime_error(
        "Transaction class not yet implemented - requires public transaction methods in Database class");
}

Transaction::~Transaction() {
    if (!committed_ && !rolled_back_) {
        try {
            rollback();
        } catch (...) {
            // Destructor must not throw
            // Log error if logging is available
        }
    }
}

void Transaction::commit() {
    if (committed_) {
        throw std::runtime_error("Transaction already committed");
    }
    if (rolled_back_) {
        throw std::runtime_error("Transaction already rolled back");
    }

    // Implementation would call: db_.commit();
    committed_ = true;
}

void Transaction::rollback() {
    if (committed_) {
        throw std::runtime_error("Cannot rollback committed transaction");
    }
    if (rolled_back_) {
        return;  // Already rolled back, no-op
    }

    // Implementation would call: db_.rollback();
    rolled_back_ = true;
}

Savepoint::Savepoint(Database& db, const std::string& name) : db_(db), name_(name) {
    if (name.empty()) {
        throw std::runtime_error("Savepoint name cannot be empty");
    }

    // Implementation would call:
    // db_.execute_raw("SAVEPOINT " + name_);
    throw std::runtime_error(
        "Savepoint class not yet implemented - requires execute_raw to be public in Database class");
}

Savepoint::~Savepoint() {
    if (!released_ && !rolled_back_) {
        try {
            release();
        } catch (...) {
            // Destructor must not throw
        }
    }
}

void Savepoint::rollback() {
    if (released_) {
        throw std::runtime_error("Cannot rollback released savepoint");
    }
    if (rolled_back_) {
        return;  // Already rolled back
    }

    // Implementation would call:
    // db_.execute_raw("ROLLBACK TO SAVEPOINT " + name_);
    rolled_back_ = true;
}

void Savepoint::release() {
    if (rolled_back_) {
        throw std::runtime_error("Cannot release rolled back savepoint");
    }
    if (released_) {
        return;  // Already released
    }

    // Implementation would call:
    // db_.execute_raw("RELEASE SAVEPOINT " + name_);
    released_ = true;
}

}  // namespace psr
