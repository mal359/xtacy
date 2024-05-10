
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "trippy.h"
#include "lmath.h"

extern XPoint PolartoRect(float,float);

void
exit_xload()
{
  return;
}

void
draw_xload(int winno)
{
  static float statOne=0.0, statFive=0.0, statFifteen=0.0;
  static float ang=0.0;
  int halfX=CX[winno]>>1;
  int halfY=CY[winno]>>1;
  int scrsize=M[winno]>>2;
  float one,five,fifteen;
  int colorone,colorfive,colorfifteen;
  XGCValues gcval;
  XPoint plotme;
   
  FILE *proc=fopen("/proc/loadavg","r");
  if(proc) 
  {
    fscanf(proc,"%f %f %f", &one, &five, &fifteen);
    fclose(proc);
  }
  else
  {
    printf("Can't open /proc/loadavg .. switching to a different mode\n");
    options.mode =(modes)rndm(lightning);
    return;
  }
/*
   ok... we've got the load info... what do we do with it?
 */
  /*
     Steve suggested a Radial Xload 
   */
  colorone=(int) (one*10)%(numcolors-1);
  colorfive=(int) (five*10)%(numcolors-1);
  colorfifteen=(int) (fifteen*10)%(numcolors-1);
  gcval.function=GXor;
  XChangeGC(display,color_gcs[colorone],GCFunction,&gcval);
  XChangeGC(display,color_gcs[colorfive],GCFunction,&gcval);
  XChangeGC(display,color_gcs[colorfifteen],GCFunction,&gcval);

  plotme=PolartoRect(one*scrsize,ang);
  XDrawLine(display,window[winno],color_gcs[colorone],
	    halfX,halfY, halfX+plotme.x,halfY+plotme.y);

  plotme=PolartoRect(five*scrsize,ang);
  XDrawLine(display,window[winno],color_gcs[colorfive],
	    halfX,halfY, halfX+plotme.x,halfY+plotme.y);

  plotme=PolartoRect(fifteen*scrsize,ang);
  XDrawLine(display,window[winno],color_gcs[colorfifteen],
	    halfX,halfY, halfX+plotme.x,halfY+plotme.y);

  ang+= (M_PI/48);
  return;
}

