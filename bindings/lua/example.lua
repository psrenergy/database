db:create_element(
    "Collection",
    {
        label = "Item 1",
        some_integer = 10,
        some_float = 1.5,
    }
);

db:create_element(
    "Collection",
    {
        label = "Item 2",
        some_integer = 20,
        some_float = 2.5,
    }
);

local labels = db:read_scalar_strings("Collection", "label"); -- { "Item 1", "Item 2" }
local integers = db:read_scalar_integers("Collection", "some_integer"); -- { 10, 20 }
local floats = db:read_scalar_doubles("Collection", "some_float"); -- { 1.5, 2.5 }
