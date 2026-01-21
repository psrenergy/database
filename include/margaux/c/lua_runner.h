#ifndef PSR_C_LUA_RUNNER_H
#define PSR_C_LUA_RUNNER_H

#include "common.h"
#include "database.h"

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle type
typedef struct margaux_lua_runner margaux_lua_runner_t;

// Create a new LuaRunner for the given database
PSR_C_API margaux_lua_runner_t* margaux_lua_runner_new(margaux_t* db);

// Destroy a LuaRunner
PSR_C_API void margaux_lua_runner_free(margaux_lua_runner_t* runner);

// Run a Lua script
// Returns PSR_OK on success, or an error code on failure.
// If an error occurs, call margaux_lua_runner_get_error() to get the error message.
PSR_C_API margaux_error_t margaux_lua_runner_run(margaux_lua_runner_t* runner, const char* script);

// Get the last error message (or NULL if no error).
// The returned pointer is valid until the next call to margaux_lua_runner_run().
// Callers should copy the string if they need to retain it beyond that.
PSR_C_API const char* margaux_lua_runner_get_error(margaux_lua_runner_t* runner);

#ifdef __cplusplus
}
#endif

#endif  // PSR_C_LUA_RUNNER_H
