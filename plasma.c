/* 
 *   Plasma Cloud and related Stuff
 *
 *   (k) 1998 Jer Johnson (jer@gweep.net)
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "trippy.h"
#include "lmath.h"

/* #define IMAGE 1 */

/* Plasma code based on OOZE.C, written by Jeff Clough for the IBM/PCs */

static void draw_grid(int,int,int,short);
static short get_pix(int,int);
static short adjust(int,int,int,int,int,int,int);
static void subdiv (int,int,int,int,int);

extern Visual *vis;
static short *grid; /* grid is a list of the colors */


#ifdef IMAGE
  XImage *im; 
#endif

void
twiddle_plasma(int winno, int x1, int y1, int x2, int y2)
{
  int clr1=grid[x1+y1*CX[winno]];
  int clr2=grid[x1+y2*CX[winno]];
  int clr3=grid[x2+y1*CX[winno]];
  int clr4=grid[x2+y2*CX[winno]];

  clr1+=(rndm(9)-4);
  clr2+=(rndm(9)-4);
  clr3+=(rndm(9)-4);
  clr4+=(rndm(9)-4);

  draw_grid(winno,x1,y1,clr1);
  draw_grid(winno,x1,y2,clr2);
  draw_grid(winno,x2,y1,clr3);
  draw_grid(winno,x2,y2,clr4);
/*  subdiv(winno,x1,y1,x2,y2,grid); */
}


void
exit_plasma()
{
  free(grid);
}


static int useCX, useCY;

/* Generate a plasma cloud */
void
draw_plasma(int winno)
{
  int x,y;

#ifdef IMAGE
  im = XGetImage(display,window[winno],0,0,CX[winno],CY[winno],~0L,ZPixmap);
  if(im==NULL)
  {
    fprintf(stderr, "Couldn't create Image. Exiting\n");
    exit(1);
  }
#endif
 

  /* make points at (0,0) (0,CY[winno]) (CX[winno],0) (CX[winno],CY[winno])
     of random colors */
 
  x=(CX[winno]-1)/(options.mirrors+1);
  y=(CY[winno]-1)/(options.mirrors+1);
  useCX=x;
  useCY=y;

#ifndef IMAGE
  /* Yow! This puppy could get HUGE */
  if ((grid = (short *)calloc((x*y),sizeof(short)))==NULL)
  {
    fprintf(stderr, "Can't Alloc enough memory for the plasma grid\n");
    exit(1);
  }
#endif

  draw_grid(winno,0,0,rndm(numcolors));
  draw_grid(winno,x,0,rndm(numcolors));
  draw_grid(winno,0,y,rndm(numcolors));
  draw_grid(winno,x,y,rndm(numcolors));
  subdiv(winno,0,0,x,y);
  if(options.mirrors)
  {
    int dx,dy,drawx,drawy;
    int x,y;
	
    for(dx=0;dx<=useCX;dx++)
      for(dy=0;dy<=useCY;dy++)
      {
	int colr=get_pix(dx,dy);
	do {
	  if(colr<0)
	    colr+=numcolors;
	  if(colr>=numcolors)
	    colr-=numcolors;
	  if(colr==0)
	    colr=1;
	} while (colr<0 || colr>numcolors);
	for (x=0;x<=options.mirrors;x++)
	  for (y=0;y<=options.mirrors;y++)
	  {
	    if(x==0 && y==0)
	      continue;
	    
	    if(x%2==0) drawx=(x*useCX)+dx;
	    else       drawx=((x+1)*useCX)-dx;
	    if(y%2==0) drawy=(y*useCY)+dy;
	    else       drawy=((y+1)*useCY)-dy;
#ifdef IMAGE
	    XPutPixel(im,drawx,drawy,colr);
#else	    
	    XDrawPoint(display,window[winno],color_gcs[colr],drawx,drawy);
#endif
	  }
      } 
  }

#ifdef IMAGE
  /*  im=XCreateImage(display,vis,DisplayPlanes(display,screen),ZPixmap,0,
		  grid,CX[winno],CY[winno],8,CX[winno]);
		  */
  
  XPutImage(display,window[winno],color_gcs[0],im,0,0,0,0,CX[winno],CY[winno]);
  /* XDestroyImage(im); */
#endif
  /*  free(grid); */
}

void
draw_grid(int winno,int x, int y, short colr)
{
#ifndef IMAGE
  grid[x+y*useCX]=colr;
#endif
  do
  {
    if(colr<0)
      colr+=numcolors;
    if(colr>=numcolors)
      colr-=numcolors;
    if(colr==0)
      colr=1;
  } while(colr<0 || colr>numcolors);
#ifndef IMAGE
  XDrawPoint(display,window[winno],color_gcs[colr],x,y);
#else
  XPutPixel(im,x,y,colr);
  /*  grid[x+y*useCX]=colr; */
#endif
}

short
get_pix(int x, int y)
{
#ifdef IMAGE
  return (short)(XGetPixel(im,x,y))%numcolors;
#else
  return grid[x+(y*useCX)];
#endif  
}

void
subdiv(int winno, int x1, int y1, int x2, int y2)
{
  int x,y;
  if (((x2-x1)<2) &&((y2-y1)<2))
    return;
 
  x=(x1+x2)/2;
  y=(y1+y2)/2;


  if (options.opt1 ==2)
  {
    short spooge;
    if((get_pix(x,y)==0)||(options.mono))
    {
      spooge=64+20*( sin(x/30) + cos(y/46) +
		     cos(x/37) + sin(y/70) +
		     sin((x+y)/43));
      
      draw_grid(winno,x,y,spooge);
      subdiv(winno,x1,y1,x,y);
      subdiv(winno,x,y1,x2,y);
      subdiv(winno,x1,y,x,y2);
      subdiv(winno,x,y,x2,y2);
    }
  }
  else
  {
    adjust(winno,x1,y1,x,y1,x2,y1);
    adjust(winno,x2,y1,x2,y,x2,y2);
    adjust(winno,x1,y2,x,y2,x2,y2);
    adjust(winno,x1,y1,x1,y,x1,y2);
    if((get_pix(x,y)==0)||(options.mono))
      {
	short spooge;
	/* Trell mode started as an experiment. it wasn't what I was expecting,
	   but cool enough to leave.. */
	if (options.trell==1) /* same as trell == opt1 */
	  spooge=(get_pix(x1,y1)+get_pix(x1,y2)+
		  get_pix(x2,y1)+get_pix(x2,y2))%numcolors;
	else
	  spooge=(get_pix(x1,y1)+get_pix(x1,y2)+
		  get_pix(x2,y1)+get_pix(x2,y2))/4;
	draw_grid(winno,x,y,spooge);
      } 
    subdiv(winno,x1,y1,x,y);
    subdiv(winno,x,y1,x2,y);
    subdiv(winno,x1,y,x,y2);
    subdiv(winno,x,y,x2,y2);
  }
}

short
adjust(int winno, int x1, int y1, int x, int y, int x2, int y2)
{
  short c;
  int d;
  int spoo;
  short clr1,clr2,clr3,clr4;

  if(!options.mono && (get_pix(x,y)!=0))
    return get_pix(x,y);

  clr1=get_pix(x1,y1);
  clr2=get_pix(x2,y2);
  clr3=get_pix(x1,y2);
  clr4=get_pix(x2,y1);
  d=math_dist(x1,y1,x2,y2);
/* leave the next 8 lines commented out for mellow colors all the time...*/
  if((spoo=rndm(100))>options.number)
  {
    if(spoo%2==1)
      c=(((clr1+clr2)/2)+rndm(d))%numcolors;
    else
      c=(((clr1+clr2)/2)-rndm(d))%numcolors;
  }
  else
    
    c=((clr1+clr2+clr3+clr4)/4); /* %numcolors; */
  if(c<0)
    c=abs(c);
  else if(c==0)
    c=1;
  draw_grid(winno,x,y,c);
  return c;
}

void
move_dogplasma(int winno)
{
  XCopyArea(display,window[winno],window[winno],color_gcs[1],
	    0,0,CX[winno]-1,CY[winno]-1,1,1);
}

void
exit_dogplasma()
{
  return;
}

void
draw_dogplasma(int winno)
{
  int x,y,clr;
  short **grid;
  grid = (short**)malloc(2*sizeof(short*));
  grid[0]= (short*) malloc(CX[winno]*sizeof(short));
  grid[1]= (short*) malloc(CX[winno]*sizeof(short));
  
  clr= rndm(numcolors);
  grid[0][0]=clr;

  for(y=0;y<CY[winno];y++)
  {
    int colr;

    if(y)
    {
      grid[y%2][0] = grid[(y-1)%2][0]+rndm(3)-1;
      colr=grid[y%2][0];
      
      do {
	if(colr<0)
	  colr+=(numcolors-1);
	if(colr>=numcolors)
	  colr-=(numcolors-1);
      } while (colr<0 || colr>numcolors);


      if(colr<=0)
	colr=1;
      XDrawPoint(display,window[winno],color_gcs[colr],0,y);
    }

    
    for(x=1;x<CX[winno];x++)
    {
      if(y)
      {
	clr= (grid[(y-1)%2][x] + grid[y%2][x-1]) >> 1;
      }
      
      if(rndm(100)> options.number)
	clr+= (rndm(3)-1);

      grid[y%2][x]=clr;
      colr=clr;

      do
      {
	if(colr<0)
	  colr+=(numcolors-1);
	if(colr>=numcolors)
	  colr-=(numcolors-1);
      } while (colr<0 || colr>numcolors);


      if(colr<=0)
	colr=1;
      
      XDrawPoint(display,window[winno],color_gcs[colr],x,y);
    }
  }

  free(grid[0]);   free(grid[1]); free(grid);
}

