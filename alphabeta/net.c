/*
 *  Grid of lines by jer johnson (jer@gweep.net)
 *  based on something I saw at a party
 */


#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <math.h>
#include "trippy.h"

void
draw_net(int winno)
{
/* determine each time the color, direction, and spacing */
  int colr, dir, xspacing,yspacing;
  int x,y,useCX=CX[winno],useCY=CY[winno];
  int midX,midY,x2;
  static int clear = 0;
  dir = rndm(50); 
  if(dir<25)
  {
    dir=-1;
    x=CX[winno];
  }
  else
  {
    dir = 1;
    x=0;
  }
  xspacing = rndm(CX[winno])+1;
  yspacing = rndm(CY[winno])+1;
  if(clear==0)
  {
    colr = 128; /* rndm(numcolors); */
    clear=1;
  }
  else
  {
    colr = 0;
    clear=0;
  }
  y=rndm(CY[winno]>>2); /* always start near the top */
  x2 = (rndm(40)>20)?0:CX[winno];

  for(x=0;x<useCX;x+=xspacing,y+=yspacing)
  {
    XDrawLine(display,window[winno],color_gcs[colr],
	      useCX-x,useCY-y,x,y);
  }
  for(y=0;y<useCY;y+=yspacing,x+=xspacing)
  {
    XDrawLine(display,window[winno],color_gcs[colr],
	      x,useCY-y,useCX-x,y);
  }
  /*
  for(x=0;x<useCX;x+=xspacing)
  {
    XDrawLine(display,window[winno],color_gcs[colr],
	      x,0,midX,midY);
  }
  for(y=0;y<useCY;y+=yspacing)
  {
    XDrawLine(display,window[winno],color_gcs[colr],
	      useCX,y,midX,midY);
  }
*/
/*
  for(;x<(dir>0?CX[winno]:0);x+=(xspacing*dir))
  {
    XDrawLine(display,window[winno],color_gcs[colr],
              x,0,x+xspacing,CY[winno]);
    y+=yspacing;
    printf("%d,%d\n",x,y);
  }
*/
  return;
}

void
exit_net()
{

}

