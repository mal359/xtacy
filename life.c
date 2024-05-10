/*
 *  The Rules to Life, as explained to me by Duane Morin
 *   If a Dead cell has 3 neighbors, it becomes alive. Otherwise, it stays
 *      dead.
 *   If a alive cell has 2 or 3 neighbors, it stays alive.
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include "trippy.h"

static short** neighbors; /* 0 to 8 , plus an extra bit to tell status*/
static short** age;      /* 0 to numcolors */
static short *xsize,*mX;
static short *ysize,*mY;

#define ON 0x0010
static void findNeighbors(int,int,int);
static void kAA(int,int,int);
static int initd=0;

void
exit_life()
{
  int i;

  initd=0;
  for(i=0;i<options.windows;i++)
  {
    free(age[i]);
    free(neighbors[i]);
  }
  free(age);
  free(neighbors);
  free(mY);
  free(mX);
  free(xsize);
  free(ysize);
}

void
getALife(int winno)
{
  int x,y;

  if(!initd)
  {
    age=(short **)calloc(options.windows,sizeof(short **));
    neighbors=(short **)calloc(options.windows,sizeof(short *));
    mX=(short*)calloc(options.windows,sizeof(short));
    mY=(short*)calloc(options.windows,sizeof(short));
    xsize=(short*)calloc(options.windows,sizeof(short));
    ysize=(short*)calloc(options.windows,sizeof(short));
    initd=1;
  }
  mX[winno]=CX[winno];
  mY[winno]=CY[winno];
  xsize[winno]=ysize[winno]=1;
  while(mX[winno]>=128)
  {
    xsize[winno]<<=1;
    mX[winno]>>=1;
  }
  while(mY[winno]>=128)
  {
    ysize[winno]<<=1;
    mY[winno]>>=1;
  }
 
  age[winno]=(short *)calloc((mX[winno]+1)*(mY[winno]+1),sizeof(short));
  neighbors[winno]=(short *)calloc((mX[winno]+1)*(1+mY[winno]),sizeof(short));
 
  for(x=0;x<mX[winno];x++)
  {
    for(y=0;y<mY[winno];y++)
    {
      if(rndm(100)<options.number)
      {
	int here=y*mX[winno]+x;
	
	neighbors[winno][here]=ON;
	XFillRectangle(display,window[winno],
		       color_gcs[age[winno][here]=1],
		       x*xsize[winno],y*ysize[winno],
		       xsize[winno],ysize[winno]);
      }
    }
  }
}

void
endLife(int winno)
{
  free(age[winno]);
  free(neighbors[winno]);
}

void
dropACell(int winno, int x,int y)
{
  x /= xsize[winno];
  y /= ysize[winno];
  neighbors[winno][y*mX[winno]+x]^=ON; /* flip it on */
}

void
drawLife(int winno)
{
  int x,y;
 
  for(y=0;y<mY[winno]+2;y++)
  {
    for(x=0;x<mX[winno];x++)
    {
      findNeighbors(x,y,winno);
      /*      findNeighbors((x+100)%(mX[winno]),y); */
      if(y-2>=0)      kAA(x,y-2,winno);
      /*      if((x+98)%(mX[winno])>0) kAA((x+98)%(mX[winno]),y); */
    }
  }
}

void
findNeighbors(int x,int y, int winno)
{
  int foo,bar;
  if(x>=mX[winno]) return;
  if(neighbors[winno][y*mX[winno]+x]&ON)
  {
    for(bar=y-1;bar<=y+1;bar++)
      for(foo=x-1;foo<=x+1;foo++)
      {
	if( (foo<0) || (bar<0) || (foo>=mX[winno]) || (bar>=mY[winno]) ||
	    (foo==x&&bar==y))
	  continue;
	else
	{
	  neighbors[winno][bar*mX[winno]+foo]++; /* tell everyone around you
						    that they have a neighbor*/
	}	   
      }
  }
}

void
kAA(int x, int y, int winno) /* it's alive... it's dead... */
{
  int nov;
  int coord = y*mX[winno]+x;
  
  if(((nov=neighbors[winno][coord]&0x000f)==3)||
     ((nov==2)&&(neighbors[winno][coord]&ON)))
  {
    neighbors[winno][coord]=ON;
    if(++age[winno][coord]>=numcolors)
      age[winno][coord]=1;
  }
  else
  {
    age[winno][coord]=0;
    neighbors[winno][coord]=0;
  }

  XFillRectangle(display,window[winno],
		 color_gcs[age[winno][coord]],
		 x*xsize[winno],y*ysize[winno],
		 xsize[winno],ysize[winno]);
}

