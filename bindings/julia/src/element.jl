struct Element
    ptr::Ptr{C.psr_element}

    function Element()
        ptr = C.psr_element_create()
        if ptr == C_NULL
            error("Failed to create Element")
        end
        obj = new(ptr)
        # finalizer(C.psr_element_destroy, obj)
        return obj
    end
end

function Base.setindex!(el::Element, value::Integer, name::String)
    cname = Base.cconvert(Cstring, name)
    err = C.psr_element_set_int(el.ptr, cname, Int32(value))
    if err != C.PSR_OK
        error("Failed to set int value for '$name'")
    end
end

function Base.setindex!(el::Element, value::Float64, name::String)
    cname = Base.cconvert(Cstring, name)
    err = C.psr_element_set_double(el.ptr, cname, value)
    if err != C.PSR_OK
        error("Failed to set double value for '$name'")
    end
end

function Base.setindex!(el::Element, value::String, name::String)
    cname = Base.cconvert(Cstring, name)
    cvalue = Base.cconvert(Cstring, value)
    err = C.psr_element_set_string(el.ptr, cname, cvalue)
    if err != C.PSR_OK
        error("Failed to set string value for '$name'")
    end
end

function Base.setindex!(el::Element, value::Vector{<:Integer}, name::String)
    if isempty(value)
        error("Empty vector not allowed for '$name'")
    end
    
    # Create a vector group with single column having same name as the group
    group_ptr = C.psr_vector_group_create()
    if group_ptr == C_NULL
        error("Failed to create vector group for '$name'")
    end
    
    try
        for val in value
            C.psr_vector_group_add_row(group_ptr)
            C.psr_vector_group_set_int(group_ptr, name, Int64(val))
        end
        
        err = C.psr_element_add_vector_group(el.ptr, name, group_ptr)
        if err != C.PSR_OK
            error("Failed to add vector group '$name' to element")
        end
    finally
        C.psr_vector_group_destroy(group_ptr)
    end
end

function Base.setindex!(el::Element, value::Vector{<:Float64}, name::String)
    if isempty(value)
        error("Empty vector not allowed for '$name'")
    end
    
    # Create a vector group with single column having same name as the group
    group_ptr = C.psr_vector_group_create()
    if group_ptr == C_NULL
        error("Failed to create vector group for '$name'")
    end
    
    try
        for val in value
            C.psr_vector_group_add_row(group_ptr)
            C.psr_vector_group_set_double(group_ptr, name, Float64(val))
        end
        
        err = C.psr_element_add_vector_group(el.ptr, name, group_ptr)
        if err != C.PSR_OK
            error("Failed to add vector group '$name' to element")
        end
    finally
        C.psr_vector_group_destroy(group_ptr)
    end
end

function Base.setindex!(el::Element, value::Vector{<:AbstractString}, name::String)
    if isempty(value)
        error("Empty vector not allowed for '$name'")
    end
    
    # Create a vector group with single column having same name as the group
    group_ptr = C.psr_vector_group_create()
    if group_ptr == C_NULL
        error("Failed to create vector group for '$name'")
    end
    
    try
        for val in value
            C.psr_vector_group_add_row(group_ptr)
            C.psr_vector_group_set_string(group_ptr, name, val)
        end
        
        err = C.psr_element_add_vector_group(el.ptr, name, group_ptr)
        if err != C.PSR_OK
            error("Failed to add vector group '$name' to element")
        end
    finally
        C.psr_vector_group_destroy(group_ptr)
    end
end

function Base.show(io::IO, e::Element)
    cstr = C.psr_element_to_string(e.ptr)
    str = unsafe_string(cstr)
    C.psr_string_free(cstr)
    print(io, str)
    return nothing
end
