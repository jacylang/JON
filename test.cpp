#include "jon/jon.h"

int main(const int argc, const char ** argv) {
    using namespace jon::literal;

    auto val = R"('asdsadasdsasddddddddddddddddddddddddddd
    )"_jon;

    auto schema = R"(
        type: 'object'
        props: {
        }
    )"_jon;

    std::cout << val.validate(schema).stringify("  ");

    return 0;
}
