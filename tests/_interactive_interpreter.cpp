#include "../include/core.hpp"
#include "../include/lex.hpp"

#include <rdesc/rdesc.h>

#include <iostream>
#include <string>
#include <utility>

using std::cin, std::cout;
using std::string;


int main() {
    auto parser = global_cfg()->new_parser();

    Lex lex { cin };

    Interpreter intr { std::move(parser) };

    enum rdesc_result res;
    while (true) {
        do {
            auto tk = lex.next();

            res = intr.pump(tk);
        } while (res == RDESC_CONTINUE);

        if (res == RDESC_NOMATCH) {
            cout << intr;
            break;
        }
    };
}
