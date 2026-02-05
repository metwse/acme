#include "../include/Xapp.hpp"
#include "../include/grammar.hpp"

#include <X11/Xlib.h>

#include <cstdlib>
#include <iostream>


using std::cerr, std::endl;


int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <simulation_file>" << endl;

        return EXIT_FAILURE;
    }

    auto grammar = global_cfg();

    XInitThreads();
    App app;

    app.init();

    return EXIT_SUCCESS;
}
