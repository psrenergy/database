"""
    query_string(db::Database, sql::String) -> Union{String, Nothing}

Execute a SQL query and return the first column of the first row as a String.
Returns `nothing` if the query returns no rows.
"""
function query_string(db::Database, sql::String)
    out_value = Ref{Ptr{Cchar}}(C_NULL)
    out_has_value = Ref{Cint}(0)

    err = C.quiver_database_query_string(db.ptr, sql, out_value, out_has_value)
    if err != C.QUIVER_OK
        throw(DatabaseException("Failed to execute query"))
    end

    if out_has_value[] == 0 || out_value[] == C_NULL
        return nothing
    end
    result = unsafe_string(out_value[])
    C.quiver_string_free(out_value[])
    return result
end

"""
    query_integer(db::Database, sql::String) -> Union{Int64, Nothing}

Execute a SQL query and return the first column of the first row as an Int64.
Returns `nothing` if the query returns no rows.
"""
function query_integer(db::Database, sql::String)
    out_value = Ref{Int64}(0)
    out_has_value = Ref{Cint}(0)

    err = C.quiver_database_query_integer(db.ptr, sql, out_value, out_has_value)
    if err != C.QUIVER_OK
        throw(DatabaseException("Failed to execute query"))
    end

    if out_has_value[] == 0
        return nothing
    end
    return out_value[]
end

"""
    query_float(db::Database, sql::String) -> Union{Float64, Nothing}

Execute a SQL query and return the first column of the first row as a Float64.
Returns `nothing` if the query returns no rows.
"""
function query_float(db::Database, sql::String)
    out_value = Ref{Float64}(0.0)
    out_has_value = Ref{Cint}(0)

    err = C.quiver_database_query_float(db.ptr, sql, out_value, out_has_value)
    if err != C.QUIVER_OK
        throw(DatabaseException("Failed to execute query"))
    end

    if out_has_value[] == 0
        return nothing
    end
    return out_value[]
end

"""
    query_date_time(db::Database, sql::String) -> Union{DateTime, Nothing}

Execute a SQL query and return the first column of the first row as a DateTime.
Returns `nothing` if the query returns no rows.
"""
function query_date_time(db::Database, sql::String)
    result = query_string(db, sql)
    if result === nothing
        return nothing
    end
    return string_to_date_time(result)
end
