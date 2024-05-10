/*
 *  Electrical Fields by Jer Johnson (jer@gweep.net)
 *
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "trippy.h"
#include "lmath.h"

typedef struct _PointCh
{
  int x,y;  /* double */
  int charge;
} PointCharge;

PointCharge **field;
static int inited=0;

void
exit_fields()
{
  int i;
  
  for(i=0;i<options.windows;i++)
  {
    free(field[i]);
  }
  free(field);
}

void
init_field(int winno)            /* I'm in a FIELD! */
{
  int i;

  if(!inited)
  {
    field=(PointCharge **)malloc(sizeof(PointCharge *)*options.windows);
    inited=1;
  }

  field[winno]=(PointCharge *)malloc(sizeof(PointCharge)* options.number);
  for(i=0;i<options.number;i++)
  {
    field[winno][i].x=rndm(CX[winno]);
    field[winno][i].y=rndm(CY[winno]);
    field[winno][i].charge=(rndm(160)-80);
/*
    fprintf(stderr,"Point %d (%d,%d) = %lf\n",i,field[winno][i].x,
	    field[winno][i].y,
	    field[winno][i].charge);
 */
  }  
}

void
draw_field(int winno)
{
  struct rect {int top,bottom,left,right;} rt;
  int i,x,y;
  rt.top = 0;
  rt.bottom = CY[winno];
  rt.left = 0;
  rt.right= CX[winno];
   
  switch(options.opt1%3)
  {
    case 1:		/* field direction */
    {
      double qx,qy,angle;
      
      for(y=rt.top;y<rt.bottom;y++)
      {
	for(x=rt.left;x<rt.right;x++)
	{
	  qx=qy=0.0;
	  for(i=0;i<options.number;i++)
	  {
	    int dx,dy,r2;
	    double r;
	    
	    dx=field[winno][i].x-x;
	    dy=field[winno][i].y-y;
	    if ((dx==0)&&(dy==0))
	      continue;
	    r2=dx*dx+dy*dy;
	    r=sqrt(r2);
	    qx+= field[winno][i].charge*10*dy/(double)(r2*r);
	    qy+= field[winno][i].charge*10*dx/(double)(r2*r);
	  }
	  angle=atan2(qx,qy)*(double)numcolors/M_PI;
	  
	  XDrawPoint(display,window[winno],
		     color_gcs[(((int)angle+numcolors)%(numcolors-1))+1],
		     x,y);
	}
      }
      break;
    }
    case 2:    /* combine the old and the new */
    {
      double q,qx,qy,angle;
      int clrRt = (int)sqrt(numcolors);   /* this looks best with pal #3 */
      int clrx,clry,clr;
      for(y=rt.top;y<rt.bottom;y++)
      {
	for(x=rt.left;x<rt.right;x++)
	{
	  q=qx=qy=0.0;
	  for(i=0;i<options.number;i++)
	  {
	    int dx,dy,r2;
	    double r;
	    
	    dx=field[winno][i].x-x;
	    dy=field[winno][i].y-y;
	    if ((dx==0)&&(dy==0))
	      continue;
	    r2=dx*dx+dy*dy;
	    r=sqrt(r2);
	    qx+= field[winno][i].charge*10*dy/(double)(r2*r);
	    qy+= field[winno][i].charge*10*dx/(double)(r2*r);
	    q+= (double)field[winno][i].charge*10 / r;
	  }
	  angle=atan2(qx,qy)*(double)numcolors/ M_PI;
	  
	  clrx=abs((int)(q+clrRt))%clrRt;
	  clry=abs((int)(angle+clrRt))%clrRt;
	  clr=(clrx+(clry*clrRt));
	  if(clr<=0) clr=1;
	  XDrawPoint(display,window[winno],color_gcs[clr],x,y);
	}
      }
      break;
    }
    case 0:		/* field amplitude */
    default:
    {
      double q;
      
      for(y=0;y<CY[winno];y++)
      {
	for(x=0;x<CX[winno];x++)
	{
	  q=0.0;
	  for(i=0;i<options.number;i++)
	  {
	    int dx,dy;
	    
	    if (((dx=x-field[winno][i].x)==0) &&
		((dy=y-field[winno][i].y)==0))
	      continue;
	    
	    q+= (double)field[winno][i].charge*10 /
	      (double)math_dist(field[winno][i].x, field[winno][i].y, x, y);
	  }
	  
	  while(q<=0)
	    q+=numcolors;
	  XDrawPoint(display,window[winno],
		     color_gcs[((int)q%(numcolors-1))+1],x,y);
	}
      }
    }
  }
}



