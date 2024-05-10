/*
 *  Play a game of Tag..  this is kind of like the Anti-Swarm
 *  instead of heading towards 'IT' .. everyone runs away from it
 *	Written by Jer (mpython@gnu.ai.mit.edu , jer@gulik.gweep.net)
 */

#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include "lmath.h"
#include "trippy.h"

#ifdef WINDOWS
extern foo options;
extern HPALETTE hPal;
#endif

#define sgn(x)  ((x)==0?0:((x)>0?1:-1))

struct kid 
{
  long x,y;
  long oldx,oldy;
  signed int vx,vy;
  signed int maxvx, maxvy;
  unsigned int color;
};

int cur_kid;

struct kid  **playground_p;
struct kid  **it;
struct kid  **it_target;
struct kid  **last_it;

static struct kid* findClosestKid(int);

void
init_it(int winno)
{

  it[winno] = &playground_p[winno][rndm(options.number)];
  last_it[winno]=0;
#ifdef WINDOWS
  it[winno]->color = GetNearestPaletteIndex(hPal,RGB(0,255,255));
#else
  it[winno]->color = 3*numcolors/6; /* CYAN */
#endif
  it_target[winno] = findClosestKid(winno);  
}

void
init_kid (int winno, struct kid *pt)
{
  pt->x = (long)rndm(CX[winno])*64;
  pt->y = (long)rndm(CY[winno])*64;
  pt->oldx = pt->x;
  pt->oldy = pt->y;
  pt->vx= pt->vy=0;
  pt->maxvx = rndm(8*64)+64;
  pt->maxvy = rndm(8*64)+64;
  
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
draw_kid (int winno, struct kid  *b)
{
  XDrawLine (display, window[winno], color_gcs[b->color], b->x/64,b->y/64,
	     b->oldx/64, b->oldy/64);
}

void
clear_kid (int winno, struct kid  *b)
{
  XDrawLine (display, window[winno], color_gcs[0], b->x/64, b->y/64,
	     b->oldx/64, b->oldy/64);
}

void
moveIt(int winno, long x, long y)
{
  clear_kid(winno,it[winno]);
  it[winno]->x = x*64;
  it[winno]->y = y*64;
  it[winno]->vx = it[winno]->vy = 0;
}

int
update_kid (int winno, struct kid * b)
{
  long x, y;
  unsigned long usecx = (long)CX[winno]*64;
  unsigned long usecy = (long)CY[winno]*64;

  clear_kid (winno, b);
  
  x = b->x; y = b->y;
  if(b==it[winno]) 
  {
    b->vx += sgn(it_target[winno]->x - x)*16;
    b->vy += sgn(it_target[winno]->y - y)*16;
  }
  else
  {
    if(math_dist(b->x,b->y,it_target[winno]->x,it_target[winno]->y)<(32*64))
    {  /* too close, run away!!! */
      b->vx += -sgn(it[winno]->x-x)*16;
      b->vy += -sgn(it[winno]->y-y)*16;
    }
    else 
    {  
      b->vx >>=1;  /* slow down by half */
      b->vy >>=1;
    }
    
#if 0
    else /* head back into the middle */
    {
      b->vx += sgn(usecx>>1-x)*16;
      b->vy += sgn(usecy>>1-y)*16;
    }
#endif
  }
  
  if(abs(b->vx)>b->maxvx) b->vx=b->maxvx*sgn(b->vx);
  if(abs(b->vy)>b->maxvy) b->vy=b->maxvy*sgn(b->vy);

  b->x += b->vx + (rndm(3)-1);
  b->y += b->vy + (rndm(3)-1);
  
/* keep everyone in the playyard */
  if(b->x<0)
  {
    b->x= 0;
    b->vx = 0;
  }
  else if (b->x>=usecx)
  {
    b->x= usecx-64;
    b->vx = 0;
  }
  if(b->y<0)
  {
    b->y= 0;
    b->vy =0;
  }
  else if(b->y>=usecy)
  {
    b->y= usecy-64;
    b->vy = 0;
  }
  
  b->oldx = x; b->oldy = y;

  draw_kid (winno, b);
  if(options.multi)
    b->color = (b->color+1)%numcolors;
  return 0;
}

int
update_it(int winno)
{
  /*
  if(!rndm(100) || (it[winno].x == it_target_x) &&
		   (it[winno].y == it_target_y))
  {
    it_target_x = (long)rndm(CX[winno])*64;
    it_target_y = (long)rndm(CY[winno])*64;
  }
  */
  it_target[winno] = findClosestKid(winno);
  
/*  update_kid(winno,&it[winno],it_target_x,it_target_y); */
  return 0;
}

struct kid*
findClosestKid(int winno) 
{
  int d = 999999,temp_d;
  int i, itnum;
  struct kid* temp=0;
 
  itnum=-1; 
  for (i=0;i<options.number;i++)
  {
    if(&playground_p[winno][i]==it[winno])
      continue;
    if(&playground_p[winno][i]==last_it[winno])
      continue;
    
    if((temp_d=math_dist(it[winno]->x,it[winno]->y,
			 playground_p[winno][i].x,playground_p[winno][i].y))<d)
    {
      d=temp_d;
      temp=&playground_p[winno][i];
      itnum=i;
      if(d<=64)  /* gotcha */
        break;  
    }
  }

  if(d<=64) /* TAG! no tagbacks */
  {
    it[winno]->color = numcolors/6; /* YELLOW */
    last_it[winno] = it[winno];
    it[winno] = temp;
    it[winno]->color = 3*numcolors/6; /* CYAN */
    it_target[winno] = &playground_p[winno][rndm(options.number)];
    fprintf(stderr,"TAG! #%d is it\n",itnum);
    return it_target[winno];
    
  }
  else
    return temp;
}

static int inited=0;

void
init_playground(int winno)
{
  if(!inited)
  {
    playground_p=(struct kid  * *) malloc((sizeof(struct kid *) *
					   options.windows));
    it = (struct kid  **) malloc((sizeof(struct kid )*options.windows));
    it_target =(struct kid  **) malloc((sizeof(struct kid )*options.windows));
    last_it =  (struct kid  **) malloc((sizeof(struct kid )*options.windows));
    inited=1;
  }

  if(options.number < 3)
  {
    options.number=3;  /* you need at least 3 to play tag */
  }
  
  
  playground_p[winno] = (struct kid  *) malloc(sizeof (struct kid ) *
					       options.number);

  for (cur_kid=0; cur_kid<options.number; cur_kid++)
  {
    init_kid (winno,&(playground_p[winno][cur_kid]));
    draw_kid (winno,&(playground_p[winno][cur_kid]));
  }
  init_it(winno);
}

void
exit_playground(int winno)
{
  if(inited)
  {
    free(playground_p[winno]);
    free(playground_p);
    free(it);
    free(it_target);
    free(last_it);
    inited=0;
  }
}

void
exit_tag()
{
  exit_playground(0);
}

void
draw_playground(int winno)
{
  for(cur_kid=0;cur_kid<options.number;cur_kid++)
  {
    update_kid(winno,&(playground_p[winno][cur_kid]));
  }
  update_it(winno);
}
