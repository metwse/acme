#include "xdriver.h"

#include <stdlib.h>


int main()
{
	xconnect();
	xinit_window();

	xdisconnect();
	return EXIT_SUCCESS;
}
