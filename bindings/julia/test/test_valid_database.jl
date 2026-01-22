module TestValidDatabaseDefinitions

using Quiver
using Test

include("fixture.jl")

@testset "Valid Schema" begin
    @testset "Basic" begin
        path_schema = joinpath(tests_path(), "schemas", "valid", "basic.sql")
        db = Quiver.from_schema(":memory:", path_schema)
        Quiver.close!(db)
        @test true
    end

    @testset "Collections" begin
        path_schema = joinpath(tests_path(), "schemas", "valid", "collections.sql")
        db = Quiver.from_schema(":memory:", path_schema)
        Quiver.close!(db)
        @test true
    end

    @testset "Relations" begin
        path_schema = joinpath(tests_path(), "schemas", "valid", "relations.sql")
        db = Quiver.from_schema(":memory:", path_schema)
        Quiver.close!(db)
        @test true
    end
end

end
