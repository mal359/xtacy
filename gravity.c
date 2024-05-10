/*  Gravity.c , by jer johnson (jer@gweep.net)
 *  Gravity well simulation
 */

/* Thanks to Andrew Petrarca (android@hotblack.gweep.net)
 * for speeding up this code
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "trippy.h"

struct well
{
  double x,y;
  int mass;
} *wells;

struct particle** partlst;
void reset_part(struct particle *,int);
static int inited=0;

void
exit_gravity()
{
  int i;
  
  free(wells);
  for(i=0;i<options.windows;i++)
  {
    free(partlst[i]);
  }
  free(partlst);

  inited=0;
}

void
init_gravity(int winno)
{
  register int i;
  
  if(!inited)
  {
    partlst= (struct particle**)malloc(options.windows*
				       sizeof(struct particle*));
    inited=1;
    wells = (struct well *)malloc(options.number*sizeof(struct well));
  }

  partlst[winno]=(struct particle *)malloc(options.numparts*
					   sizeof(struct particle));
  
  /* set up the gravity wells */
  for(i=0;i<options.number;i++)
  {
    /* weight the wells closer to the center */
    wells[i].x=rndm(CX[winno]/2)+rndm(CX[winno]/2);
    wells[i].y=rndm(CY[winno]/2)+rndm(CY[winno]/2);
    wells[i].mass=1000;
    XDrawPoint(display,window[winno],color_gcs[1],(int)wells[i].x,
              (int)wells[i].y);
  }
/* and init the particles */
  
  for(i=0;i<options.numparts;i++)
  {
    reset_part(&partlst[winno][i],winno);      
  }
}

void
set_part(int winno,int x,int y,struct particle *thingy)
{
  thingy->x=x;
  thingy->y=y;
  thingy->mass=rndm(20);
  thingy->vx=thingy->vy=0.0;
  thingy->color=rndm(numcolors);
}

void
reset_part(struct particle *thingy,int winno)
{
  thingy->x=rndm(CX[winno]);
  thingy->y=rndm(CY[winno]);
  thingy->mass=rndm(20);
  thingy->vx=(double)rndm(101)/1000.0-0.05;
  thingy->vy=(double)rndm(101)/1000.0-0.05;
/*  thingy->vx=thingy->vy=0.0; */
  thingy->color=rndm(numcolors);
}


int
draw_gravity(int winno)
{
  register int i,j;
  double ax,ay;

  for(j=0;j<options.numparts;j++)
  {
    struct particle *thingy =&partlst[winno][j];
    ax=ay=0.0;
    for(i=0;i<options.number;i++)
    {
      double xoff,yoff,r2,r,farce;
      /* do the forces from each well */
      xoff=wells[i].x-thingy->x;
      yoff=wells[i].y-thingy->y;
      r2=xoff*xoff+yoff*yoff;
      r=sqrt(r2);
      farce=((double)6.67e-4*(double)wells[i].mass)/(r2*r);

      ax+=xoff*farce;
      ay+=yoff*farce;
    }
  
    for(i=j+1;i<options.numparts;i++)
    {
      double xoff,yoff,r2,r,farce;
      if(thingy==&partlst[winno][i])
	continue;

      /* do the forces from each particle */
      xoff=partlst[winno][i].x-thingy->x;
      yoff=partlst[winno][i].y-thingy->y;
      r2=xoff*xoff+yoff*yoff;
      r=sqrt(r2);
      farce=((double)6.67e-4)/(r2*r);
      ax+=xoff*farce*(double)partlst[winno][i].mass;
      ay+=yoff*farce*(double)partlst[winno][i].mass;
      partlst[winno][i].vx-=xoff*farce*(double)thingy->mass;
      partlst[winno][i].vy-=yoff*farce*(double)thingy->mass;
    }
       
    /* move the particle */
    thingy->vx+=ax;
    thingy->vy+=ay;
    thingy->x+=thingy->vx;
    thingy->y+=thingy->vy;

    if(options.multi)
      thingy->color= (thingy->color+1)%numcolors;
    
    XDrawPoint(display,window[winno], color_gcs[thingy->color],(int)thingy->x,
	       (int)thingy->y);

    if((thingy->x>CX[winno])||(thingy->y>CY[winno])||
       (thingy->x<0)||(thingy->y<0))
      reset_part(thingy,winno);
  }
  return 0;
}

