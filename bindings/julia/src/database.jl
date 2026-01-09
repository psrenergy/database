struct Database
    ptr::Ptr{C.psr_database}
end

function create_empty_db_from_schema(db_path, schema_path; force::Bool = true)
    options = Ref(C.psr_database_options_t(0, C.PSR_LOG_DEBUG))
    ptr = C.psr_database_from_schema(db_path, schema_path, options)
    if ptr == C_NULL
        throw(DatabaseException("Failed to create database from schema '$schema_path'"))
    end
    return Database(ptr)
end

function create_element!(db::Database, collection::String, e::Element)
    if C.psr_database_create_element(db.ptr, collection, e.ptr) == -1
        throw(DatabaseException("Failed to create element in collection $collection"))
    end
    return nothing
end

# Helper to get set table info for a collection
function _get_set_table_info(db::Database, collection::String)
    out_count = Ref{Int64}(0)
    tables_ptr = C.psr_database_get_set_tables(db.ptr, collection, out_count)
    
    if tables_ptr == C_NULL || out_count[] == 0
        return Dict{String, Vector{NamedTuple{(:name, :fk_table), Tuple{String, Union{String, Nothing}}}}}()
    end
    
    result = Dict{String, Vector{NamedTuple{(:name, :fk_table), Tuple{String, Union{String, Nothing}}}}}()
    
    try
        for i in 1:out_count[]
            table_name = unsafe_string(unsafe_load(tables_ptr, i))
            # Extract group name from table name (Collection_set_groupname -> groupname)
            prefix = "$(collection)_set_"
            group_name = table_name[length(prefix)+1:end]
            
            # Get columns for this set table
            col_count = Ref{Int64}(0)
            cols_ptr = C.psr_database_get_table_columns(db.ptr, table_name, col_count)
            
            if cols_ptr != C_NULL && col_count[] > 0
                columns = Vector{NamedTuple{(:name, :fk_table), Tuple{String, Union{String, Nothing}}}}()
                try
                    for j in 1:col_count[]
                        col_info = unsafe_load(cols_ptr, j)
                        col_name = unsafe_string(col_info.name)
                        fk_table = col_info.fk_table == C_NULL ? nothing : unsafe_string(col_info.fk_table)
                        push!(columns, (name=col_name, fk_table=fk_table))
                    end
                finally
                    C.psr_column_info_array_free(cols_ptr, col_count[])
                end
                result[group_name] = columns
            end
        end
    finally
        C.psr_string_array_free(tables_ptr, out_count[])
    end
    
    return result
end

# Helper to find element ID by label
function _find_element_id(db::Database, collection::String, label::String)
    id = C.psr_database_find_element_id(db.ptr, collection, label)
    if id == -1
        throw(DatabaseException("Element with label '$label' not found in collection '$collection'"))
    end
    return id
end

function create_element!(db::Database, collection::String; kwargs...)
    e = Element()
    
    # Get set table info for this collection
    set_tables = _get_set_table_info(db, collection)
    
    # Collect all set column names
    set_columns_to_group = Dict{String, String}()  # column_name -> group_name
    set_columns_fk = Dict{String, Union{String, Nothing}}()  # column_name -> fk_table or nothing
    for (group_name, columns) in set_tables
        for col in columns
            set_columns_to_group[col.name] = group_name
            set_columns_fk[col.name] = col.fk_table
        end
    end
    
    # Separate kwargs into scalars, vectors, and set columns
    set_kwargs = Dict{String, Any}()
    other_kwargs = Dict{Symbol, Any}()
    
    for (k, v) in kwargs
        key_str = String(k)
        if haskey(set_columns_to_group, key_str) && v isa Vector
            set_kwargs[key_str] = v
        else
            other_kwargs[k] = v
        end
    end
    
    # Set non-set attributes on element
    for (k, v) in other_kwargs
        e[String(k)] = v
    end
    
    # Build set groups
    # Group set_kwargs by their group name
    group_data = Dict{String, Dict{String, Vector}}()
    for (col_name, values) in set_kwargs
        group_name = set_columns_to_group[col_name]
        if !haskey(group_data, group_name)
            group_data[group_name] = Dict{String, Vector}()
        end
        group_data[group_name][col_name] = values
    end
    
    # Create set groups
    for (group_name, columns) in group_data
        # Validate all columns have same length
        lengths = [length(v) for v in values(columns)]
        if !allequal(lengths)
            throw(DatabaseException("All set columns in group '$group_name' must have the same length"))
        end
        if isempty(lengths) || first(lengths) == 0
            throw(DatabaseException("Empty set values not allowed for group '$group_name'"))
        end
        
        num_rows = first(lengths)
        
        # Create set group via C API
        set_group_ptr = C.psr_set_group_create()
        if set_group_ptr == C_NULL
            throw(DatabaseException("Failed to create set group"))
        end
        
        try
            for row_idx in 1:num_rows
                C.psr_set_group_add_row(set_group_ptr)
                
                for (col_name, values) in columns
                    val = values[row_idx]
                    fk_table = set_columns_fk[col_name]
                    
                    # If column is a FK and value is a string, resolve to ID
                    if fk_table !== nothing && val isa AbstractString
                        val = _find_element_id(db, fk_table, val)
                    end
                    
                    # Set the value
                    if val isa Integer
                        C.psr_set_group_set_int(set_group_ptr, col_name, Int64(val))
                    elseif val isa Float64
                        C.psr_set_group_set_double(set_group_ptr, col_name, val)
                    elseif val isa AbstractString
                        C.psr_set_group_set_string(set_group_ptr, col_name, val)
                    else
                        throw(DatabaseException("Unsupported value type for set column '$col_name': $(typeof(val))"))
                    end
                end
            end
            
            # Add set group to element
            err = C.psr_element_add_set_group(e.ptr, group_name, set_group_ptr)
            if err != C.PSR_OK
                throw(DatabaseException("Failed to add set group '$group_name' to element"))
            end
        finally
            C.psr_set_group_destroy(set_group_ptr)
        end
    end
    
    create_element!(db, collection, e)
    return nothing
end

function close!(db::Database)
    C.psr_database_close(db.ptr)
    return nothing
end

function read_scalar_parameters(db::Database, collection::String, attribute::String; default=nothing)
    # Try double first (most common for numeric data)
    out_values = Ref{Ptr{Cdouble}}(C_NULL)
    count = C.psr_database_read_scalar_parameters_double(db.ptr, collection, attribute, out_values)

    if count >= 0
        try
            result = Vector{Float64}(undef, count)
            for i in 1:count
                val = unsafe_load(out_values[], i)
                if isnan(val) && default !== nothing
                    result[i] = Float64(default)
                else
                    result[i] = val
                end
            end
            return result
        finally
            C.psr_double_array_free(out_values[])
        end
    end

    # Try string
    out_strings = Ref{Ptr{Ptr{Cchar}}}(C_NULL)
    count = C.psr_database_read_scalar_parameters_string(db.ptr, collection, attribute, out_strings)

    if count >= 0
        try
            result = Vector{String}(undef, count)
            for i in 1:count
                str_ptr = unsafe_load(out_strings[], i)
                result[i] = unsafe_string(str_ptr)
            end
            return result
        finally
            C.psr_string_array_free(out_strings[], count)
        end
    end

    # Try int
    out_ints = Ref{Ptr{Int64}}(C_NULL)
    count = C.psr_database_read_scalar_parameters_int(db.ptr, collection, attribute, out_ints)

    if count >= 0
        try
            result = Vector{Int64}(undef, count)
            for i in 1:count
                result[i] = unsafe_load(out_ints[], i)
            end
            return result
        finally
            C.psr_int_array_free(out_ints[])
        end
    end

    throw(DatabaseException("Failed to read scalar parameters '$attribute' from collection '$collection'"))
end

function read_scalar_parameter(db::Database, collection::String, attribute::String, label::String)
    # Try double first
    out_value = Ref{Cdouble}(0.0)
    is_null = Ref{Cint}(0)
    err = C.psr_database_read_scalar_parameter_double(db.ptr, collection, attribute, label, out_value, is_null)

    if err == C.PSR_OK
        return is_null[] != 0 ? NaN : out_value[]
    elseif err == C.PSR_ERROR_NOT_FOUND
        throw(DatabaseException("Element with label '$label' not found in collection '$collection'"))
    end

    # Try string
    out_str = Ref{Ptr{Cchar}}(C_NULL)
    err = C.psr_database_read_scalar_parameter_string(db.ptr, collection, attribute, label, out_str)

    if err == C.PSR_OK
        try
            return unsafe_string(out_str[])
        finally
            C.psr_string_free(out_str[])
        end
    elseif err == C.PSR_ERROR_NOT_FOUND
        throw(DatabaseException("Element with label '$label' not found in collection '$collection'"))
    end

    # Try int
    out_int = Ref{Int64}(0)
    is_null = Ref{Cint}(0)
    err = C.psr_database_read_scalar_parameter_int(db.ptr, collection, attribute, label, out_int, is_null)

    if err == C.PSR_OK
        return is_null[] != 0 ? 0 : out_int[]
    elseif err == C.PSR_ERROR_NOT_FOUND
        throw(DatabaseException("Element with label '$label' not found in collection '$collection'"))
    end

    throw(DatabaseException("Failed to read scalar parameter '$attribute' for '$label' from collection '$collection'"))
end
