/* started 9-6-90 by Matthew Cross
 *
 * Displays a moving star background on your X-display, to make it appear
 * as if your windows are moving in space....
 *
 *  WINDOWS IN SPAAAAAAAAAAAACE!!!!!
 *
 *  Dragged kicking and screaming into Xtacy 2/21/94 jpj
 *
 *  And mostly finished 5/2/94 ( yup, it's a real background job)
 */

#include <X11/Xlib.h>
#include <X11/X.h>
#include <stdlib.h>
#include "trippy.h"

extern XColor *color_info;

/* global defines, D_ means default */

#define D_STAR_SPEED 20  /* calculated as follows...
			   new_x = old_x + (old_x - x_center) / star_speed;
			   new_y = old_y + (old_y - y_center) / star_speed;
			 */
#define D_TURN_SPEED 18
#define D_USLEEP_TIME 10
#define D_N_STARS     100
#define D_INIT_RANGE  100
#define HIGHPREC 32
#define WAIT 1000
#define D_MAX_IND_SPEED 30
#define D_RANDOM_CHANCE 50000
#define MAX_PIXELS 5000

/* global vars */

int star_speed = D_STAR_SPEED,
    usleep_time = D_USLEEP_TIME,
    init_range = D_INIT_RANGE,
    turn_speed = D_TURN_SPEED,
    max_ind_speed = D_MAX_IND_SPEED;
long random_chance = D_RANDOM_CHANCE;
int n_pixels;

int cur_star=0;

struct point
{
  long x, y;
  long oldx,oldy;
  int ind_speed;
  int pixel;
};

struct point **star_array_p;
int center_x, center_y;
int wanted_center_x, wanted_center_y;
int root_w, root_h;
short def_root, finish = 0, moveable = 0, random_move = 1, multi_color = 0;
static int inited=0;

static void init_point(struct point*);

#define draw_star(winno,star)  XDrawLine (display, window[winno], color_gcs[(star)->pixel], (star)->x/HIGHPREC, (star)->y/HIGHPREC, (star)->oldx/HIGHPREC, (star)->oldy/HIGHPREC)
#define clear_star(winno,star) XDrawLine (display, window[winno], color_gcs[0], (star)->x/HIGHPREC, (star)->y/HIGHPREC, (star)->oldx/HIGHPREC, (star)->oldy/HIGHPREC)

void
init_point (struct point *pt)
{
  pt->x = rndm(init_range * 2);
  pt->y = rndm(init_range * 2);
  pt->x += center_x - init_range;
  pt->y += center_y - init_range;
  pt->x *= HIGHPREC;
  pt->y *= HIGHPREC;
  pt->oldx = pt->x;
  pt->oldy = pt->y;
  pt->ind_speed = (rndm(max_ind_speed)) + 1;
  if(options.multi)
    pt->pixel = rndm(numcolors);
  else
    pt->pixel=1; /* WHITE */
}

void
init_galaxy(int winno)
{
  if(!inited)
  {
    star_array_p=(struct point **) malloc (sizeof(struct point*) *
					   options.windows);
    inited=1;
    if(!options.multi)
    {
      make_white(1);  /* make the stars white */
    }
  }
  
  star_array_p[winno] = (struct point *) malloc (sizeof (struct point) *
						 options.number);
  wanted_center_x=center_x= CX[winno]>>1;
  wanted_center_y=center_y= CY[winno]>>1;
  root_w=CX[winno];
  root_h=CY[winno];

  for (cur_star=0; cur_star<options.number; cur_star++)
  {
    init_point (&(star_array_p[winno][cur_star]));
    draw_star (winno, &(star_array_p[winno][cur_star]));
  }
}


int
update_star (int winno,struct point *star)
{
  long x, y;

  clear_star (winno, star);
  x = star->x; y = star->y;
  star->x = x + (x - center_x*HIGHPREC)/(star_speed + star->ind_speed);
  star->y = y + (y - center_y*HIGHPREC)/(star_speed + star->ind_speed);
  star->oldx = x; star->oldy = y;
  if(star->oldx == star->x)
  {
    star->x++;
  }
  if(star->oldy == star->y)
  {
    star->y++;
  }
  if(options.multi)
  {
    star->pixel= (star->pixel+1)%numcolors;

  }
  if ((star->oldx/HIGHPREC < 0) || (star->oldx/HIGHPREC > root_w) ||
      (star->oldy/HIGHPREC < 0) || (star->oldy/HIGHPREC > root_h))
  {
    return 1;
  }
  else
  {
    draw_star (winno, star);
  }
  return 0;
}

void
update_center (int winno)
{
  int x,y;

  if(options.opt1 != 0)
  {
    Window root,child;
    unsigned int mask=0;
    int rx=0,ry=0,wx=0,wy=0;
    XQueryPointer(display,window[winno],&root,&child,
    		  &rx,&ry,&wx,&wy,&mask);

    if(options.doroot)
    {
      wanted_center_x = rx; wanted_center_y = ry;
    }
    else
    {
      wanted_center_x = wx; wanted_center_y = wy;
    }
  }


  x = center_x; y = center_y;
  center_x = x + (wanted_center_x - x)/(turn_speed);
  if(center_x<0) center_x=0;
  if(center_x>CX[winno]) center_x= CX[winno];
  center_y = y + (wanted_center_y - y)/(turn_speed);
  if(center_y<0) center_y=0;
  if(center_y>CX[winno]) center_y=CY[winno];
  return;
}

void
exit_starback()
{
  int i;
  
  for(i=0;i<options.windows;i++)
  {
    free(star_array_p[i]);
  }
  free(star_array_p);
  inited=0;
}


void
draw_stars(int winno)
{
  for(cur_star=0;cur_star<options.number;cur_star++)
  {
    if (update_star (winno,&(star_array_p[winno][cur_star])))
    {
      init_point (&(star_array_p[winno][cur_star]));
      draw_star (winno, &(star_array_p[winno][cur_star]));
    }
  }
  
  if (((wanted_center_x != center_x)  ||
       (wanted_center_y != center_y)) ||
      options.opt1 !=0)
    update_center (winno);
  
  if ((random_move) && ((rndm(random_chance) == 0)))
  {
    wanted_center_x = rndm(root_w);
    wanted_center_y = rndm(root_h);
  }
}

