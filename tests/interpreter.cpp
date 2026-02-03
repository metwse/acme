#include "../src/core.hpp"
#include "../src/lex.hpp"

#include <rdesc/rdesc.h>

#include <iostream>
#include <string>
#include <utility>

using std::cin, std::cout, std::endl, std::flush;
using std::string;


int main() {
    auto cfg = load_grammar();
    auto parser = cfg->new_parser();

    Lex lex { cin };

    Interpreter intr { std::move(parser) };
    cout << "> " << flush;

    enum rdesc_result res;
    while (true) {
        do {
            auto tk = lex.next();

            res = intr.pump(tk);
        } while (res == RDESC_CONTINUE);

        if (res == RDESC_READY) {
            cout << "> " << flush;
        } else {
            cout << intr << endl;
            break;
        }
    };

}
