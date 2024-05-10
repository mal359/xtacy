
/* module for making a blank cursor */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "trippy.h"
#include "vroot.h"

Cursor mycursor;		/* blank cursor */
static Pixmap lockc;
static Pixmap lockm;		/* pixmaps for cursor and mask */
static char no_bits[] = {0};	/* dummy array for the blank cursor */
static Window root;

void
makeBlankCursor()
{
  XColor nullcolor;

  root=RootWindow(display, screen);

  lockc = XCreateBitmapFromData(display, root, no_bits, 1, 1);
  lockm = XCreateBitmapFromData(display, root, no_bits, 1, 1);
  mycursor = XCreatePixmapCursor(display, lockc, lockm,
				 &nullcolor, &nullcolor, 0, 0);
  XFreePixmap(display, lockc);
  XFreePixmap(display, lockm);
}


