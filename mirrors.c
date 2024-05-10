/*
 *  Mirrors.c   by Jer Johnson (jer@gweep.net)
 *
 *
 */

/*  Pass in:  a point to be mirrored , how many mirrors to make
 *  Return:  a list of points mirrored around the center
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lmath.h"
#include "trippy.h"

#define EVENP(x)  (((x)%1)==0)

XPoint*
mirror(int winno, int x, int y, int n, int mode)
{
  XPoint *ptr;
  int i;
  const int centx = CX[winno]>>1;
  const int centy = CY[winno]>>1;

  if(n==0) n=1;

  ptr = (XPoint*)malloc(sizeof(XPoint)*n);

  ptr[0].x = x;
  ptr[0].y = y;

  switch (n) {
  case 1:
	return ptr;  /* we're done */
#if 0
  case 8:
    {
      ptr[5].x = ptr[0].y;
      ptr[5].y = CY[winno] - ptr[0].x;
      
      ptr[6].x = ptr[5].x;
      ptr[6].y = CY[winno] - ptr[5].y;
    
    } /* fall thru */
  case 4:
    {
      /* do the horizontal */
      ptr[(n/2)].x = CX[winno] - ptr[0].x;
      ptr[(n/2)].y = CY[winno] - ptr[0].y;
      ptr[n-1].x = ptr[0].x;
      ptr[n-1].y = CY[winno] - ptr[0].y;
      
      mode = MIR_VERT; /* and let this do the VIRT */
      break;
    }
#endif
  case 2:
    {
  
      break;
    }
  default:
    {
      /*  do funky stuff with trig and stuff */
      double steps;
      int dx,dy,dist;
      double angle;
	  
      steps = 2* M_PI / n;

      dx = x - centx;
      dy = y - centy;
      dist = math_dist(0,0,dx,dy);
      angle = (RecttoPolar(dx, dy));
      for(i=1; i< n; i++)
	{
	  
	  angle += steps;
	  ptr[i] = PolartoRect(dist,angle);
	  ptr[i].x += centx;
	  ptr[i].y += centy;
	}
      
      return ptr;
      break;
    }
  }
 
  if(EVENP(n))
    {
      if(n == 2)
	{
	  i=1;
	  if((mode & MIR_VERT) == MIR_VERT)
	    {
	      ptr[i].x = CX[winno] - ptr[0].x;
	      ptr[i].y = ptr[0].y;
	    }
	      
	  if((mode & MIR_HORIZ) == MIR_HORIZ)
	    {
	      ptr[i].x = ptr[0].x;
	      ptr[i].y = CY[winno] - ptr[0].y;
	    }

	  /* printf("mode = %d i=%d ptr=%d,%d\n",mode,i,ptr[i].x,ptr[i].y); */
	}
      else
	{
	  for(i=1;i<(n/2);i++)
	    {
	      if((mode & MIR_VERT) == MIR_VERT)
		{
		  ptr[i].x = CX[winno] - ptr[i+(n/2)].x;
		  ptr[i].y = ptr[i+(n/2)].y;
		}
	      
	      if((mode & MIR_HORIZ) == MIR_HORIZ)
		{
		  ptr[i].x = ptr[i+(n/2)].x;
		  ptr[i].y = CY[winno] - ptr[i+(n/2)].y;
		}

	  /*    printf("i=%d ptr=%d,%d\n",i,ptr[i].x,ptr[i].y); */
	    }
	}
    }
  return ptr;
}

