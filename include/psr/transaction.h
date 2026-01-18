#ifndef PSR_TRANSACTION_H
#define PSR_TRANSACTION_H

#include "export.h"

namespace psr {

class Database;

/**
 * @brief RAII transaction guard for database operations
 * 
 * Automatically begins a transaction on construction and rolls back
 * on destruction unless explicitly committed. This ensures that
 * transactions are properly handled even in the presence of exceptions.
 * 
 * @example
 * @code
 * Database db("test.db");
 * 
 * // Automatic rollback on exception
 * {
 *     Transaction txn(db);
 *     db.create_element("Collection", Element().set("label", "Item1"));
 *     db.update_element("Collection", 1, Element().set("value", 42));
 *     txn.commit();  // Explicit commit
 * }  // Auto-rollback if commit() not called
 * 
 * // Using in exception-safe code
 * try {
 *     Transaction txn(db);
 *     db.create_element("Collection", Element().set("label", "Item2"));
 *     throw std::runtime_error("Something went wrong");
 *     txn.commit();  // Never reached
 * } catch (...) {
 *     // Transaction automatically rolled back
 * }
 * @endcode
 */
class PSR_API Transaction {
public:
    /**
     * @brief Begins a new transaction
     * 
     * @param db The database to start transaction on
     * @throws std::runtime_error if transaction cannot be started
     */
    explicit Transaction(Database& db);

    /**
     * @brief Destructor - automatically rolls back if not committed
     * 
     * This ensures that uncommitted transactions are rolled back,
     * preventing partial updates in case of exceptions.
     */
    ~Transaction();

    // Non-copyable
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    // Non-movable (to prevent confusion about ownership)
    Transaction(Transaction&&) = delete;
    Transaction& operator=(Transaction&&) = delete;

    /**
     * @brief Commits the transaction
     * 
     * After calling commit(), the destructor will not roll back.
     * 
     * @throws std::runtime_error if commit fails
     */
    void commit();

    /**
     * @brief Explicitly rolls back the transaction
     * 
     * After calling rollback(), the destructor will not attempt rollback again.
     * This can be useful for explicit error handling.
     */
    void rollback();

    /**
     * @brief Checks if transaction has been committed
     * 
     * @return true if commit() was called successfully
     */
    bool is_committed() const { return committed_; }

    /**
     * @brief Checks if transaction has been rolled back
     * 
     * @return true if rollback() was called or destructor rolled back
     */
    bool is_rolled_back() const { return rolled_back_; }

    /**
     * @brief Checks if transaction is still active (not committed or rolled back)
     * 
     * @return true if transaction is active
     */
    bool is_active() const { return !committed_ && !rolled_back_; }

private:
    Database& db_;
    bool committed_ = false;
    bool rolled_back_ = false;
};

/**
 * @brief Savepoint within a transaction
 * 
 * Allows creating nested transaction-like behavior using savepoints.
 * Rolling back to a savepoint undoes changes made since the savepoint
 * was created, without affecting earlier changes in the transaction.
 * 
 * @example
 * @code
 * Transaction txn(db);
 * db.create_element("Collection", Element().set("label", "Item1"));
 * 
 * {
 *     Savepoint sp(db, "sp1");
 *     db.create_element("Collection", Element().set("label", "Item2"));
 *     sp.rollback();  // Only "Item2" is rolled back
 * }
 * 
 * db.create_element("Collection", Element().set("label", "Item3"));
 * txn.commit();  // Commits "Item1" and "Item3", but not "Item2"
 * @endcode
 */
class PSR_API Savepoint {
public:
    /**
     * @brief Creates a savepoint with the given name
     * 
     * @param db The database to create savepoint on
     * @param name The savepoint name (must be unique within transaction)
     * @throws std::runtime_error if savepoint cannot be created
     */
    Savepoint(Database& db, const std::string& name);

    /**
     * @brief Destructor - automatically releases savepoint
     */
    ~Savepoint();

    // Non-copyable, non-movable
    Savepoint(const Savepoint&) = delete;
    Savepoint& operator=(const Savepoint&) = delete;
    Savepoint(Savepoint&&) = delete;
    Savepoint& operator=(Savepoint&&) = delete;

    /**
     * @brief Rolls back to this savepoint
     * 
     * Undoes all changes made since the savepoint was created.
     * 
     * @throws std::runtime_error if rollback fails
     */
    void rollback();

    /**
     * @brief Releases the savepoint without rolling back
     * 
     * This makes the changes permanent (within the transaction).
     */
    void release();

private:
    Database& db_;
    std::string name_;
    bool released_ = false;
    bool rolled_back_ = false;
};

}  // namespace psr

#endif  // PSR_TRANSACTION_H
