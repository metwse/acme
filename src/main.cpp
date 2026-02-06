#include "../include/Xapp.hpp"
#include "../include/Xdraw.hpp"
#include "../include/grammar.hpp"
#include "../include/rdesc.hpp"
#include "../include/lex.hpp"
#include "../include/interpreter.hpp"

#include <X11/Xlib.h>
#include <rdesc/rdesc.h>

#include <memory>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

using std::cerr, std::endl;
using std::ifstream, std::ios_base;
using std::string;
using std::make_shared;


int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <simulation_file>" << endl;

        return EXIT_FAILURE;
    }

    ifstream file(argv[1], ios_base::in);

    auto lex = make_shared<Lex>(file);
    auto intr = make_shared<Interpreter>(global_cfg()->new_parser());

    enum rdesc_result res;
    while (true) {
        auto tk = lex->next();

        if (tk.id == TK_NOTOKEN) {
            string line;
            std::getline(file, line);
            cerr << "Syntax error near: \n" << line << endl;

            return EXIT_FAILURE;
        } else if (tk.id == TK_EOF) {
            break;
        }

        res = intr->pump(tk);

        if (res == RDESC_NOMATCH)
            cerr << "Syntax error, ignoring a statement" << endl;
    };

    XInitThreads();
    App app { intr, lex };

    app.init();

    return EXIT_SUCCESS;
}
