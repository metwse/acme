// internal statics/function for xdriver

#ifndef XDETAIL_H
#define XDETAIL_H


#include <X11/Xlib.h>


extern Display *dpy;
extern Screen *scr;
extern Window root;

extern Window win;


void xinit_grahpics();
void xcleanup_grahpics();

void xredraw();


#endif
