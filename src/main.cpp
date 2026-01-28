#include "Xapp.hpp"

#include <X11/Xlib.h>

#include <cstdlib>


int main() {
    XInitThreads();
    App app;

    app.init();

    return EXIT_SUCCESS;
}
