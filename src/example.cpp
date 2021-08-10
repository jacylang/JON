#include "jon/jon.h"

using jon = jacylang::jon;

int main(const int, const char**) {
    using namespace jacylang::literal;

    auto file = jon::fromFile("../../examples/sample_1.jon", true);

    std::cout << file.typeStr() << std::endl;

//    auto val = file.at("value");
//    auto schema = file.at("schema");
//
//    std::cout << val.validate(schema).dump("  ") << std::endl;

    return 0;
}
