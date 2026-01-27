#include "xdriver.h"
#include "xdetail.h"
#include "detail.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <threads.h>

#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


Display *dpy;
Screen *scr;
Window root;

Window win;

static bool quit = false;
static thrd_t evthread;


static int evloop([[maybe_unused]] void *arg)
{
	Atom wm_delete_win = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wm_delete_win, 1);

	XSelectInput(dpy, win, ExposureMask);

	XEvent ev;
	while (!quit) {
		XNextEvent(dpy, &ev);

		switch (ev.type) {
		case Expose:
			xredraw();
			break;

		case ClientMessage:
			if (cast(Atom, ev.xclient.data.l[0]) == wm_delete_win)
				quit = true;
			break;

		default: unreachable();  // GCOVR_EXCL_LINE
		}
	}

	return 0;
}

void xconnect()
{
	dpy = XOpenDisplay(NULL);

	assert(dpy, "Cannot open X11 display");

	scr = XDefaultScreenOfDisplay(dpy);
	root = XRootWindowOfScreen(scr);
}

void xinit_windows()
{
	Colormap border_color = XBlackPixelOfScreen(scr);
	Colormap background_color = XWhitePixelOfScreen(scr);

	int x = XWidthOfScreen(scr) / 2 - WWIDTH / 2;
	int y = XHeightOfScreen(scr) / 2 - WHEIGHT / 2;

	win = XCreateSimpleWindow(dpy, root, x, y, WWIDTH, WHEIGHT, 1,
				  border_color, background_color);


	XSetNormalHints(dpy, win, &(XSizeHints) {
		.flags = PMinSize,
		.min_width = WWIDTH,
		.min_height = WHEIGHT,
	});

	XSetWMName(dpy, win, &(XTextProperty) {
		.value = cast(unsigned char *, "acme-sim"),
		.format = 8, .nitems = 8,
		.encoding = XUTF8StringStyle,
	});

	XMapWindow(dpy, win);
	xinit_grahpics();
	xredraw();

	// from now on, no X11 call should be done in main thread
	thrd_create(&evthread, evloop, NULL);
}

void xdisconnect()
{
	thrd_join(evthread, NULL);

	xcleanup_grahpics();

	XCloseDisplay(dpy);
}
