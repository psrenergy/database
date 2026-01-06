#include <gtest/gtest.h>
#include <psr/element.h>

TEST(Element, DefaultEmpty) {
    psr::Element builder;
    EXPECT_FALSE(builder.has_scalars());
    EXPECT_FALSE(builder.has_vectors());
    EXPECT_TRUE(builder.scalars().empty());
    EXPECT_TRUE(builder.vectors().empty());
}

TEST(Element, SetInt) {
    psr::Element builder;
    builder.set("count", int64_t{42});

    EXPECT_TRUE(builder.has_scalars());
    EXPECT_EQ(builder.scalars().size(), 1);
    EXPECT_EQ(std::get<int64_t>(builder.scalars().at("count")), 42);
}

TEST(Element, SetDouble) {
    psr::Element builder;
    builder.set("value", 3.14);

    EXPECT_TRUE(builder.has_scalars());
    EXPECT_EQ(std::get<double>(builder.scalars().at("value")), 3.14);
}

TEST(Element, SetString) {
    psr::Element builder;
    builder.set("label", std::string{"Plant 1"});

    EXPECT_TRUE(builder.has_scalars());
    EXPECT_EQ(std::get<std::string>(builder.scalars().at("label")), "Plant 1");
}

TEST(Element, SetNull) {
    psr::Element builder;
    builder.set_null("empty");

    EXPECT_TRUE(builder.has_scalars());
    EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(builder.scalars().at("empty")));
}

TEST(Element, SetVectorInt) {
    psr::Element builder;
    builder.set_vector("ids", std::vector<int64_t>{1, 2, 3});

    EXPECT_TRUE(builder.has_vectors());
    auto& vec = std::get<std::vector<int64_t>>(builder.vectors().at("ids"));
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[2], 3);
}

TEST(Element, SetVectorDouble) {
    psr::Element builder;
    builder.set_vector("costs", std::vector<double>{1.5, 2.5, 3.5});

    EXPECT_TRUE(builder.has_vectors());
    auto& vec = std::get<std::vector<double>>(builder.vectors().at("costs"));
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[1], 2.5);
}

TEST(Element, SetVectorString) {
    psr::Element builder;
    builder.set_vector("names", std::vector<std::string>{"a", "b", "c"});

    EXPECT_TRUE(builder.has_vectors());
    auto& vec = std::get<std::vector<std::string>>(builder.vectors().at("names"));
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], "a");
}

TEST(Element, FluentChaining) {
    psr::Element builder;
    builder.set("label", std::string{"Plant 1"})
           .set("capacity", 50.0)
           .set("id", int64_t{1})
           .set_vector("costs", std::vector<double>{1.0, 2.0, 3.0});

    EXPECT_EQ(builder.scalars().size(), 3);
    EXPECT_EQ(builder.vectors().size(), 1);
}

TEST(Element, Clear) {
    psr::Element builder;
    builder.set("label", std::string{"test"})
           .set_vector("data", std::vector<double>{1.0});

    EXPECT_TRUE(builder.has_scalars());
    EXPECT_TRUE(builder.has_vectors());

    builder.clear();

    EXPECT_FALSE(builder.has_scalars());
    EXPECT_FALSE(builder.has_vectors());
}

TEST(Element, OverwriteValue) {
    psr::Element builder;
    builder.set("value", 1.0);
    builder.set("value", 2.0);

    EXPECT_EQ(builder.scalars().size(), 1);
    EXPECT_EQ(std::get<double>(builder.scalars().at("value")), 2.0);
}
