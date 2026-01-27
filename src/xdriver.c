#include "xdriver.h"
#include "detail.h"  // IWYU pragma: keep

#include <X11/X.h>
#include <stdbool.h>
#include <threads.h>

#include <X11/Xlib.h>


static Display *dpy;
static Screen *scr;
static Window root;
static Window win;

static Colormap white;
static Colormap black;

static GC gc;

static bool quit = false;
static thrd_t evthread;


static void setup_gc()
{
	gc = XDefaultGCOfScreen(scr);
}

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

void xinit_window()
{
	black = XBlackPixelOfScreen(scr);
	white = XWhitePixelOfScreen(scr);

	int x = XWidthOfScreen(scr) / 2 - WWIDTH / 2;
	int y = XHeightOfScreen(scr) / 2 - WHEIGHT / 2;

	win = XCreateSimpleWindow(dpy, root, x, y, WWIDTH, WHEIGHT, 1,
				  black, white);

	XMapWindow(dpy, win);
	setup_gc();

	// from now on, no X11 call should be done in main thread
	thrd_create(&evthread, evloop, NULL);
}

void xdisconnect()
{
	thrd_join(evthread, NULL);

	XCloseDisplay(dpy);
}
