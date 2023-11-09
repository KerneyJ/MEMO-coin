#include <array>
#include <string>

struct Foo {
    int a;
    char b;
    std::string c;
};

enum status {
    OKAY, BAD, NUL
};

struct Bar {
    status type;
    std::array<Foo, 3> foos;
};