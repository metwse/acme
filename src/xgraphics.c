#include "xdriver.h"
#include "xdetail.h"
#include "detail.h"  // IWYU pragma: keep

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>


static enum {
	WELCOME,
	EDIT,
	NORMAL,
} mode;


static GC default_gc;
static Visual *visual;
static Colormap cmap;

static XftFont *font;
static XftDraw *xft_draw;
static XftColor xft_black;


static void welcome_ui()
{
	XftDrawString8(xft_draw, &xft_black, font, 8, 24,
		       (const FcChar8 *) "Welcome to acme!", 16);
}


void xredraw()
{
	switch (mode) {
	case WELCOME:
		welcome_ui();
		return;
	case EDIT:
	case NORMAL:
		break;
	}
}

void xinit_grahpics()
{
	default_gc = XDefaultGCOfScreen(scr);
	visual = XDefaultVisualOfScreen(scr);
	cmap = XDefaultColormapOfScreen(scr);

	font = XftFontOpenName(dpy, root, WFONT);
	xft_draw = XftDrawCreate(dpy, win, visual, cmap);
	XftColorAllocName(dpy, visual, cmap, "black", &xft_black);

	assert(font, "Could not open font!");
}

void xcleanup_grahpics()
{
	XftDrawDestroy(xft_draw);
	XftFontClose(dpy, font);
}
