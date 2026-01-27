#include "xdriver.h"

#include <stdlib.h>


int main()
{
	xconnect();
	xinit_windows();

	xdisconnect();
	return EXIT_SUCCESS;
}
