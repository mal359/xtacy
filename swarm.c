#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include "trippy.h"

#ifdef WINDOWS
extern foo options;
extern HPALETTE hPal;
#endif

#define sgn(x)  ((x)==0?0:((x)>0?1:-1))
/* #define sgn(x)  ((x)>0?1:-1) */

struct bee 
{
  long x,y;
  long oldx,oldy;
  signed int vx,vy;
  signed int maxvx, maxvy;
  unsigned int color;
};

int cur_bee;

struct bee  **swarm_p;
struct bee  *queen;
static unsigned long queen_target_x, queen_target_y;

void
init_queen(int winno)
{
  queen[winno].oldx= queen[winno].x = (long)rndm(CX[winno])*64;
  queen[winno].oldy= queen[winno].y = (long)rndm(CY[winno])*64;
#ifdef WINDOWS
  queen[winno].color = GetNearestPaletteIndex(hPal,RGB(255,0,255));
#else
  queen[winno].color = 5*numcolors/6; /* PURPLE */
#endif
  queen[winno].vx=queen[winno].vy=0;
  queen[winno].maxvx = queen[winno].maxvy = 8*64;
  queen_target_x = (long)rndm(CX[winno])*64;
  queen_target_y = (long)rndm(CY[winno])*64;
}

void
init_bee (int winno, struct bee *pt)
{
  pt->x = (long)rndm(CX[winno])*64;
  pt->y = (long)rndm(CY[winno])*64;
  pt->oldx = pt->x;
  pt->oldy = pt->y;
  pt->vx= pt->vy=0;
  pt->maxvx = pt->maxvy = rndm(3*64)+(7*64);
/*  pt->maxvy = rndm(8*64)+64; */
  
  if(options.multi)
    pt->color = rndm(numcolors);
  else
#ifdef WINDOWS
    pt->color=GetNearestPaletteIndex(hPal,RGB(255,255,0));
#else
    pt->color=numcolors/6; /* YELLOW */
#endif
}

void
draw_bee (int winno, struct bee  *b)
{
  XDrawLine (display, window[winno], color_gcs[b->color], b->x/64,b->y/64,
	     b->oldx/64, b->oldy/64);
}

void
clear_bee (int winno, struct bee  *b)
{
  XDrawLine (display, window[winno], color_gcs[0], b->x/64, b->y/64,
	     b->oldx/64, b->oldy/64);
}

void
moveQueen(int winno, long x, long y)
{
  clear_bee(winno,&queen[winno]);
  queen[winno].x = x*64;
  queen[winno].y = y*64;
}

void
moveQueenTarg(long x, long y)
{
  queen_target_x= x*64;
  queen_target_y= y*64;
}

int
update_bee (int winno, struct bee * b, long targx, long targy)
{
  long x, y;
  unsigned long usecx = (long)CX[winno]*64;
  unsigned long usecy = (long)CY[winno]*64;

  clear_bee (winno, b);
  
  x = b->x; y = b->y;
  b->vx += (sgn(targx-x)*16) + (rndm(16)-8);
  b->vy += (sgn(targy-y)*16) + (rndm(16)-8);
  if(abs(b->vx)>b->maxvx) b->vx=b->maxvx*sgn(b->vx); 
  if(abs(b->vy)>b->maxvy) b->vy=b->maxvy*sgn(b->vy);

  b->x += b->vx;
  b->y += b->vy;
  b->oldx = x; b->oldy = y;

  if(b->x<0)
    b->x= 0;
  else if (b->x>usecx)
    b->x= usecx;
  if(b->y<0)
    b->y= 0; 
  else if(b->y>usecy)
    b->y= usecy;

  draw_bee (winno, b);
  if(options.multi)
    b->color = (b->color+1)%numcolors;
  return 0;
}

int
update_queen(int winno)
{
  if(!rndm(100) || ((abs(queen[winno].x-queen_target_x)<64) &&
		   (abs(queen[winno].y-queen_target_y)<64)))
  {
    queen_target_x = (long)rndm(CX[winno])*64;
    queen_target_y = (long)rndm(CY[winno])*64;
  }
  
  update_bee(winno,&queen[winno],queen_target_x,queen_target_y);
  return 0;
}

static int inited=0;

void
exit_swarm()
{
  free(swarm_p[0]);
  free(swarm_p);
  free(queen);
  inited=0;
}

void
init_swarm(int winno)
{
  if(!inited)
  {
    swarm_p=(struct bee  * *) malloc((sizeof(struct bee *) * options.windows));
    queen = (struct bee  *) malloc((sizeof(struct bee )*options.windows));
    inited=1;
  }
  
  swarm_p[winno] = (struct bee  *) malloc(sizeof (struct bee ) * options.number);
  init_queen(winno);
  
  for (cur_bee=0; cur_bee<options.number; cur_bee++)
  {
    init_bee (winno,&(swarm_p[winno][cur_bee]));
    draw_bee (winno,&(swarm_p[winno][cur_bee]));
  }
}


void
draw_swarm(int winno)
{
  for(cur_bee=0;cur_bee<options.number;cur_bee++)
  {
    update_bee(winno,&(swarm_p[winno][cur_bee]),queen[winno].x,queen[winno].y);
  }
  update_queen(winno);
}

