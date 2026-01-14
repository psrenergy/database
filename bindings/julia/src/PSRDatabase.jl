module PSRDatabase

include("c_api.jl")
import .C

include("exceptions.jl")
include("element.jl")
include("database.jl")

export Element, Database, DatabaseException
export create_empty_db_from_schema, create_element!, close!
export read_scalar_ints, read_scalar_doubles, read_scalar_strings

end
