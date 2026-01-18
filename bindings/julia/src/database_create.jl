function create_element!(db::Database, collection::String, e::Element)
    if C.psr_database_create_element(db.ptr, collection, e.ptr) == -1
        throw(DatabaseException("Failed to create element in collection $collection"))
    end
    return nothing
end

function create_element!(db::Database, collection::String; kwargs...)
    e = Element()
    for (k, v) in kwargs
        e[String(k)] = v
    end
    try
        create_element!(db, collection, e)
    finally
        destroy!(e)
    end
    return nothing
end
