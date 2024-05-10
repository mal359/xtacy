/*
 *  Frame.c defines the bouncing shapes (qix,bozo, bez, etc) routines
 *  by Jer Johnson (jer@gweep.net)
 *
 */

#include <math.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "trippy.h"
#include "lmath.h"

struct pt
{
  int x, y;
  int dx,dy;
  int pixel;
  struct pt *next, *prev;
};

typedef struct pt PT;

struct PTList
{
  int size;
  PT *next, *prev;
};
typedef struct PTList ptList;

ptList *head1,*head2,*head3,*head4;

int HCV;
static int inited=0;
static XPoint *make_bozo(int,int,int,int,int,int,int,int);

void
add(ptList *here,int init,int winno)
{
  PT *point;
  
  point=(PT *)malloc(sizeof(PT));
  
  if(init)  /* if we need to create a point, do it here */
  {
    point->x=rndm(CX[winno]);
    point->y=rndm(CY[winno]);
    point->dx=1;
    point->dy=1;
    point->pixel=rndm(numcolors-1)+1;
  }
  else     /* otherwise, just move an existing point */
  {
    int radius = math_dist(head2[winno].next->x,head2[winno].next->y,
			   head1[winno].next->x,head1[winno].next->y);
    point->x=here->next->x+here->next->dx;
    point->y=here->next->y+here->next->dy;
    point->dx=here->next->dx;
    point->dy=here->next->dy;
    point->pixel=(here->next->pixel+HCV)%numcolors;
    if (!rndm((long)(1000)))
      point->pixel=rndm(numcolors-1)+1;
    if((point->x<0)|| ((options.mode==circ)&&(point->x<radius)))
    {
      point->dx=rndm(7);
      point->x+=point->dx;
    }
    else if((point->x>CX[winno])||
	    ((options.mode==circ)&&(CX[winno]-point->x<radius)))
    {
      point->dx= -rndm(7);
      point->x+=point->dx;
    }
    if((point->y<0)|| ((options.mode==circ)&&(point->y<radius)))
    {
      point->dy=rndm(7);
      point->y+=point->dy;
    }
    else if((point->y>CY[winno])||
	    ((options.mode==circ)&&(CY[winno]-point->y<radius)))
    {
      point->dy= -rndm(7);
      point->y+=point->dy;
    }
  }
  
  if(here->prev==NULL)
    here->prev=point;
  if(here->next==NULL)
    here->next=point;
  else
  {
    here->next->prev=point;
    point->next=here->next;
    point->prev=here->prev;
    here->next=point;
  }
  here->size++;
}

void delete1(ptList *here)
{
  PT *temp=here->prev->prev;
  here->next->prev=temp;
  temp->next=here->next;
  
  free(here->prev);
  here->prev=temp;
  
  here->size--;
}

void
exit_frame(int winno)
{
  int i;

  i = winno;
  fprintf(stderr,"Exiting frame\n");
  
/*  for(i=0;i<options.windows;i++) */
  {
    while(head1[i].size>0)
      delete1(&head1[i]);
    while(head2[i].size>0)
      delete1(&head2[i]);
    if(options.mode==bozo || options.mode==bez)
    {
      while(head3[i].size>0)
	delete1(&head3[i]);
      while(head4[i].size>0)
	delete1(&head4[i]);
    }
  }
  
  free(head1);
  free(head2);
  if(options.mode==bozo || options.mode==bez)
  {
    free(head3);
    free(head4);
  }
  inited=0;
}

void
exit_bozo(int winno)
{
  exit_frame(winno);
}

void
exit_circ(int winno)
{
  exit_frame(winno);
}

void
exit_qix(int winno)
{
  exit_frame(winno);
}

void
exit_qix4(int winno)
{
  exit_frame(winno);
}

void
exit_bez(int winno)
{
  exit_frame(winno);
}

void
init_frame(int winno)
{
/* make the first level */

/*  fprintf(stderr,"Init'ing frame\n"); */
  
  /* 4 lists for each window, one for each point */

  if(!inited)
  {
    
    head1=(ptList *)calloc(options.windows,sizeof(ptList));
    head2=(ptList *)calloc(options.windows,sizeof(ptList));
    if(options.mode==bozo || options.mode==bez)
    {
	head3=(ptList *)calloc(options.windows,sizeof(ptList));
	head4=(ptList *)calloc(options.windows,sizeof(ptList));
    }
    inited=1;
  }
  
/* and now the second level */

/* init the 4 points */
  head1[winno].prev=NULL;
  head1[winno].next=NULL;
  head1[winno].size=0;
  head2[winno].prev=NULL;
  head2[winno].next=NULL;
  head2[winno].size=0;
  if(options.mode==bozo || options.mode==bez)
  {
    head3[winno].prev=NULL;
    head3[winno].next=NULL;
    head3[winno].size=0;
    head4[winno].prev=NULL;
    head4[winno].next=NULL;
    head4[winno].size=0;
  }
  add(&head1[winno],1,winno);
  add(&head2[winno],1,winno);
  if(options.mode==bozo || options.mode==bez)
  {
    add(&head3[winno],1,winno);
    add(&head4[winno],1,winno);
  }
  
  if(options.multi)
    HCV=1;
  else
    HCV=0;  
}

void
move_frame(int winno)
{
  if(head1[winno].size>=options.number)
    delete1(&head1[winno]);
  if(head2[winno].size>=options.number)
    delete1(&head2[winno]);
  if(options.mode==bozo || options.mode==bez)
  {
    if(head3[winno].size>=options.number)
      delete1(&head3[winno]);
    if(head4[winno].size>=options.number)
      delete1(&head4[winno]);
  }
}

void
bounce(int winno)
{ 
  /*#*#* Bouncing Code *#*#*/
  add(&head1[winno],0,winno);
  add(&head2[winno],0,winno);
  if(options.mode==bozo|| options.mode==bez)
  {
    add(&head3[winno],0,winno);
    add(&head4[winno],0,winno);
  }
}

void
draw_qix(int winno)
{ 
  XPoint *ptr1,*ptr2,*ptr3,*ptr4;
  int i;
  
  ptr1 = mirror(winno, head1[winno].next->x, head1[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr2 = mirror(winno, head2[winno].next->x, head2[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr3 = mirror(winno, head1[winno].prev->x, head1[winno].prev->y,
		options.mirrors+1, options.opt1);
  ptr4 = mirror(winno, head2[winno].prev->x, head2[winno].prev->y,
	 	options.mirrors+1, options.opt1);
  
  for (i=0;i<=options.mirrors;i++)
  {
    XDrawLine(display,window[winno],color_gcs[head1[winno].next->pixel],
	      ptr1[i].x, ptr1[i].y, ptr2[i].x,ptr2[i].y);
    
    XDrawLine(display,window[winno],color_gcs[options.mono],
	      ptr3[i].x, ptr3[i].y, ptr4[i].x,ptr4[i].y);
    
  }

  free(ptr1);
  free(ptr2);
  free(ptr3);
  free(ptr4);
}

void
draw_qix4(int winno)
{ 

  XDrawLine(display,window[winno],color_gcs[head1[winno].next->pixel],
	    head1[winno].next->x,head1[winno].next->y,
	    head2[winno].next->x,head2[winno].next->y);
  XDrawLine(display,window[winno],color_gcs[head1[winno].next->pixel],
	    CX[winno]-head1[winno].next->x,head1[winno].next->y,
	    CX[winno]-head2[winno].next->x,head2[winno].next->y);
  XDrawLine(display,window[winno],color_gcs[head1[winno].next->pixel],
	    head1[winno].next->x,CY[winno]-head1[winno].next->y,
	    head2[winno].next->x,CY[winno]-head2[winno].next->y);
  XDrawLine(display,window[winno],color_gcs[head1[winno].next->pixel],
	    CX[winno]-head1[winno].next->x,CY[winno]-head1[winno].next->y,
	    CX[winno]-head2[winno].next->x,CY[winno]-head2[winno].next->y);

  XDrawLine(display,window[winno],color_gcs[options.mono],
	    head1[winno].prev->x,head1[winno].prev->y,
	    head2[winno].prev->x,head2[winno].prev->y);
  XDrawLine(display,window[winno],color_gcs[options.mono],
	    CX[winno]-head1[winno].prev->x,head1[winno].prev->y,
	    CX[winno]-head2[winno].prev->x,head2[winno].prev->y);
  XDrawLine(display,window[winno],color_gcs[options.mono],
	    head1[winno].prev->x,CY[winno]-head1[winno].prev->y,
	    head2[winno].prev->x,CY[winno]-head2[winno].prev->y);
  XDrawLine(display,window[winno],color_gcs[options.mono],
	    CX[winno]-head1[winno].prev->x,CY[winno]-head1[winno].prev->y,
	    CX[winno]-head2[winno].prev->x,CY[winno]-head2[winno].prev->y);
}

void
draw_bozo(int winno)
{
  /* draw the bozogon (actually just a quadrilateral,
  ** but that's not as fun a name as 'bozogon' )
  */
  XPoint *ptr1,*ptr2,*ptr3,*ptr4,*ptr5,*ptr6,*ptr7,*ptr8;
  int i;
  
  ptr1 = mirror(winno, head1[winno].next->x, head1[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr2 = mirror(winno, head2[winno].next->x, head2[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr3 = mirror(winno, head1[winno].prev->x, head1[winno].prev->y,
		options.mirrors+1, options.opt1);
  ptr4 = mirror(winno, head2[winno].prev->x, head2[winno].prev->y,
	 	options.mirrors+1, options.opt1);
  ptr5 = mirror(winno, head3[winno].next->x, head3[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr6 = mirror(winno, head4[winno].next->x, head4[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr7 = mirror(winno, head3[winno].prev->x, head3[winno].prev->y,
		options.mirrors+1, options.opt1);
  ptr8 = mirror(winno, head4[winno].prev->x, head4[winno].prev->y,
	 	options.mirrors+1, options.opt1);

  for(i=0; i<=options.mirrors; i++)
  {
    XDrawLines(display,window[winno],color_gcs[head1[winno].next->pixel],
	     make_bozo(ptr1[i].x,ptr1[i].y,
		       ptr2[i].x,ptr2[i].y,
		       ptr5[i].x,ptr5[i].y,
		       ptr6[i].x,ptr6[i].y),
	     5,CoordModeOrigin);
  /* and erase the last one */
    XDrawLines(display,window[winno],color_gcs[options.mono],
	     make_bozo(ptr3[i].x,ptr3[i].y,
		       ptr4[i].x,ptr4[i].y,
		       ptr7[i].x,ptr7[i].y,
		       ptr8[i].x,ptr8[i].y),
	     5,CoordModeOrigin);
  }
  free(ptr1);
  free(ptr2);
  free(ptr3);
  free(ptr4);
  free(ptr5);
  free(ptr6);
  free(ptr7);
  free(ptr8);
}

void
draw_bez(int winno)
{
  /* draw a Bezier curve (4 control points, 21 points on the curve */
  int m,i,clr;
  XPoint *ptr1,*ptr2,*ptr3,*ptr4,*ptr5,*ptr6,*ptr7,*ptr8;

  ptr1 = mirror(winno, head1[winno].next->x, head1[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr2 = mirror(winno, head2[winno].next->x, head2[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr3 = mirror(winno, head1[winno].prev->x, head1[winno].prev->y,
		options.mirrors+1, options.opt1);
  ptr4 = mirror(winno, head2[winno].prev->x, head2[winno].prev->y,
	 	options.mirrors+1, options.opt1);
  ptr5 = mirror(winno, head3[winno].next->x, head3[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr6 = mirror(winno, head4[winno].next->x, head4[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr7 = mirror(winno, head3[winno].prev->x, head3[winno].prev->y,
		options.mirrors+1, options.opt1);
  ptr8 = mirror(winno, head4[winno].prev->x, head4[winno].prev->y,
	 	options.mirrors+1, options.opt1);

  for (m=0; m<=options.mirrors;m++) 
  {
    XPoint* bezi;
    bezi = Bezier(make_bozo(ptr1[m].x,ptr1[m].y,
			    ptr2[m].x,ptr2[m].y,
			    ptr5[m].x,ptr5[m].y,
			    ptr6[m].x,ptr6[m].y),21);
    
    clr=head1[winno].next->pixel;

    for(i=0;i<20;i++)
    {
      if(options.opt1 == 1)
	XDrawPoint(display, window[winno], color_gcs[clr],
		   bezi[i].x,bezi[i].y);
      else
	XDrawLine(display, window[winno], color_gcs[clr],
		  bezi[i].x, bezi[i].y, bezi[i+1].x, bezi[i+1].y);
      if(options.multi)
	clr = (clr+1)%numcolors;
    }
    
    /* and erase the last one */
    bezi =  Bezier(make_bozo(ptr3[m].x,ptr3[m].y,
			     ptr4[m].x,ptr4[m].y,
			     ptr7[m].x,ptr7[m].y,
			     ptr8[m].x,ptr8[m].y),21);
    
    if(options.opt1 == 1)
      XDrawPoints(display,window[winno],color_gcs[options.mono],bezi,21,
		  CoordModeOrigin);
    else 
      XDrawLines(display,window[winno],color_gcs[options.mono],bezi,21,
		 CoordModeOrigin);
    
  }
  free(ptr1);
  free(ptr2);
  free(ptr3);
  free(ptr4);
  free(ptr5);
  free(ptr6);
  free(ptr7);
  free(ptr8);
}


void
draw_circ(int winno)
{
  XPoint *ptr1,*ptr2,*ptr3,*ptr4;
  int i;

  ptr1 = mirror(winno, head1[winno].next->x, head1[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr2 = mirror(winno, head2[winno].next->x, head2[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr3 = mirror(winno, head1[winno].prev->x, head1[winno].prev->y,
		options.mirrors+1, options.opt1);
  ptr4 = mirror(winno, head2[winno].prev->x, head2[winno].prev->y,
	 	options.mirrors+1, options.opt1);

  for(i=0;i<= options.mirrors; i++)
  {
    XDrawArc(display,window[winno],color_gcs[head1[winno].next->pixel],
	     ptr1[i].x,ptr1[i].y,
	     abs(ptr2[i].x-ptr1[i].x),
	     abs(ptr2[i].y-ptr1[i].y),
	     0, 64*360);


    /* and erase the last one */
    XDrawArc(display,window[winno],color_gcs[options.mono],
	     ptr3[i].x,ptr3[i].y,
	     abs(ptr4[i].x-ptr3[i].x),
	     abs(ptr4[i].y-ptr3[i].y),
	     0, 64*360);
  }

  free(ptr1);
  free(ptr2);
  free(ptr3);
  free(ptr4);
}

void
draw_boxes(int winno)
{ 
  XPoint *ptr1,*ptr2,*ptr3,*ptr4;
  int i;

  ptr1 = mirror(winno, head1[winno].next->x, head1[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr2 = mirror(winno, head2[winno].next->x, head2[winno].next->y,
		options.mirrors+1, options.opt1);
  ptr3 = mirror(winno, head1[winno].prev->x, head1[winno].prev->y,
		options.mirrors+1, options.opt1);
  ptr4 = mirror(winno, head2[winno].prev->x, head2[winno].prev->y,
	 	options.mirrors+1, options.opt1);

  /* draw the rectangles */
/*
   I USED to be able to just call 'XDrawRectangle', but apparently X11R5's
   version of that function doesn't deal with negative widths and heights.
   So I'm using XDrawLines and an extra XDrawLine *sigh*. If your
   XDrawRectangle works, uncomment them and use 'em
*/
  for(i=0; i<= options.mirrors; i++)
  {
/*
 XDrawRectangle(display,window[winno],color_gcs[head1[winno].next->pixel],
 head1[winno].next->x,head1[winno].next->y,
 head2[winno].next->x-head1[winno].next->x,
 head2[winno].next->y-head1[winno].next->y );
*/
    XDrawLines(display,window[winno],color_gcs[head1[winno].next->pixel],
	       wrecked(ptr1[i].x, ptr1[i].y, ptr2[i].x, ptr2[i].y),
	       4,CoordModeOrigin);
    XDrawLine(display,window[winno],color_gcs[head1[winno].next->pixel],
	      ptr1[i].x, ptr1[i].y, ptr1[i].x, ptr2[i].y);
  
    /* and erase the last one */
/*
  XDrawRectangle(display,window[winno],color_gcs[options.mono],
  head1[winno].prev->x,head1[winno].prev->y,
  head2[winno].prev->x-head1[winno].prev->x,
  head2[winno].prev->y-head1[winno].prev->y );
*/
    XDrawLines(display,window[winno],color_gcs[options.mono],
	       wrecked(ptr3[i].x, ptr3[i].y, ptr4[i].x, ptr4[i].y),
	       4,CoordModeOrigin);
    XDrawLine(display,window[winno],color_gcs[options.mono],
	      ptr3[i].x,ptr3[i].y,
	      ptr3[i].x,ptr4[i].y);
  }
  free(ptr1);
  free(ptr2);
  free(ptr3);
  free(ptr4);
}

XPoint *
make_bozo(int x1,int y1, int x2, int y2,
	  int x3,int y3, int x4, int y4)
{
  static XPoint temp[5];
  
  temp[0].x = x1; temp[0].y = y1;
  temp[1].x = x2; temp[1].y = y2;
  temp[2].x = x3; temp[2].y = y3;
  temp[3].x = x4; temp[3].y = y4;
  temp[4].x = x1; temp[4].y = y1;
  
  return temp;
}
