/*
 *  Lunchtime!  written by Jer Johnson (mpython@gnu.ai.mit.edu)
 *
 *  dedicated to Josh Brandt (mute@gweep.net) who always forgets to
 *  take lunch until 3PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include "trippy.h"
#include "lmath.h"

#define NUMEFFECTS 2

static void draw_lunch_0(int,int*);
static void draw_lunch_1(int);

void
init_lunch()
{

}

void
draw_lunch(int winno,int* n)
{
  /* different effects here */
  switch (options.opt1%NUMEFFECTS)
  {
    case 0:
    {
      draw_lunch_0(winno,n);
      break;
    }
    case 1:
    {
      draw_lunch_1(winno);
      break;
    }
/*
    case 2:
    {
      draw_lunch_2(winno);
      break;
    }
*/
    default:
    {
      draw_lunch_0(winno,n);
      break;
    }
  }
}

void
draw_lunch_0(int winno,int *n)  /* do some distance to line drawing */
{
  
  if(*n==0)
  {
    int i,x,y;
    /* build the lines */
    int x1[] = {50 ,50 ,90 ,90 ,120,140,140,160,190,190,190,245,245,280 };
    int y1[] = {50 ,150,100,150,100,100,100,100,100,100,150,75, 100,100 };
    int x2[] = {50 ,75, 90, 120,120,140,160,160,220,190,220,245,280,280 };
    int y2[] = {150,150,150,150,150,150,100,150,100,150,150,150,100,150 };


    for(x=0;x<CX[winno];x++)
      for(y=0;y<CY[winno];y++)
    {
      unsigned int d=999999;
      for (i=0;i<(sizeof(x1)/sizeof(x1[0]));i++)
      {
        unsigned int dp;
        if((dp=line_dist(x,y,((CX[winno]>>1)+x1[i]-160),
			 ((CY[winno]>>1)+y1[i]-100),
			 ((CX[winno]>>1)+x2[i]-160),
			 ((CY[winno]>>1)+y2[i]-100)))<d)
	  d=dp;
      }
      if(d==0) d=1;
      XDrawPoint(display,window[winno],color_gcs[d%numcolors],x,y);
    }
    *n=1;
  }
  else
    rotate_colors();
  
  return;
}

void
draw_lunch_1(int winno) /* whoooshy text */
{
  char *LunchStrs[] = {
    "Lunch",
    "LunchTime!",
    "Time for Lunch",
    "Nosh Nosh Nosh! Don't Stop Eating!",
    "EAT THIS!"
  };
  
  char *LunchStr=LunchStrs[rndm(5)];
  
  XDrawString (display, window[winno], color_gcs[rndm(numcolors)],
	       rndm(CX[winno]),rndm(CY[winno]),LunchStr,strlen(LunchStr));

  return;
}

void
draw_lunch_2(int winno) /* ummm.. do other stuff */
{

}

void
exit_lunch()	/* HUWAARRRF! */
{

}
