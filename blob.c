/*
 *  Wandering blobbies.. written by Jer (jer@gweep.net)
 *
 *
 */

#include <math.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "lmath.h"
#include "trippy.h"
#include <assert.h>

#define sgn(x)  ((x)==0?0:((x)>0?1:-1))

struct pt
{
  int x, y;
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

static void addPoint(ptList*,int,int,int);
static void delete1(ptList*);
static void deletePoint(ptList *,PT *);
static void drawBlob(ptList*,int,int);
static PT* getClosestFood(ptList*,ptList*,int);

#define NUMCELLS 48
#define NUMFOOD  20

static ptList ***headlist;

static ptList **foodlist;

void
exit_blob()
{
  int j,k;
  
  for(j=0;j<options.windows;j++)
  {
    for(k=0; k<options.number; k++ )
    {
      while(headlist[j][k]->size>0)
	delete1(headlist[j][k]);
      free(headlist[j][k]);
    }
    free(headlist[j]);
    
    while(foodlist[j]->size>0)
      delete1(foodlist[j]);
    free(foodlist[j]);
  }
  free(headlist);
  free(foodlist);

}

void
addPoint(ptList *here,int x, int y,int winno)
{
  PT *point;
  static int clr = 1; 
  point=(PT *)malloc(sizeof(PT));

  /*  assert(point!=0); */
  point->x=x;
  point->y=y;
  if(options.multi)
  {
    point->pixel=clr++;
    clr=clr%(numcolors-1);
  } 
  else
  {
    point->pixel=2;
  }

  if((x==-1)&&(y==-1))
  {
    if(here->prev && here->next)
    {  /* keep the blob coherent */
      point->x =((here->prev->x + here->next->x)/2) + (rndm(3)-1);
      point->y =((here->prev->y + here->next->y)/2) + (rndm(3)-1);
    }
    else
    {
      point->x = rndm(CX[winno]);
      point->y = rndm(CY[winno]);
    }
  }
  else
  {
    point->x=x;
    point->y=y;
  }
    
  if(here->next==NULL)
    here->next=point;
  if(here->prev==NULL)
    here->prev=point;
  
  here->prev->next = point;
  point->prev=here->prev;
  here->prev=point;
  
  point->next=here->next;
  here->next->prev=point;
  
  here->size++;
}

void
delete1(ptList *here)
{
  PT *temp=here->prev->prev;
  here->next->prev=temp;
  temp->next=here->next;
  
  free(here->prev);
  here->prev=temp;
  
  here->size--;
}

void
deletePoint(ptList *here,PT *thisone)
{
  thisone->next->prev=thisone->prev;
  thisone->prev->next=thisone->next;
  if(here->next == thisone)
  {
    here->next = thisone->next;
  }
  if(here->prev == thisone)
  {
    here->prev = thisone->prev;
  }
  free(thisone);
  here->size--;
}

void
initBlob(int winno)
{
  int i,j,k;

  headlist = (ptList***)malloc(sizeof(ptList**)*options.windows);
  foodlist = (ptList**)malloc(sizeof(ptList*)*options.windows);
 
  for(j=0;j<options.windows;j++)
  {
    headlist[j]=(ptList**)malloc(sizeof(ptList*)*options.number);
    for(k=0; k<options.number; k++ )
    {
      headlist[j][k]=(ptList*)malloc(sizeof(ptList));
      headlist[j][k]->prev=NULL;
      headlist[j][k]->next=NULL;
      headlist[j][k]->size=0;
    
      for(i=0;i<NUMCELLS;i++)
      {
        addPoint(headlist[j][k],-1,-1, winno);
      }
    }
    foodlist[j]=(ptList*)malloc(sizeof(ptList));
    foodlist[j]->prev=NULL;
    foodlist[j]->next=NULL;
    foodlist[j]->size=0;

   for(i=0;i<NUMFOOD;i++)
   {
     addPoint(foodlist[j],rndm(CX[winno]),rndm(CY[winno]),winno);
   }
  }
}

PT*
getClosestFood(ptList* hunter,ptList* food,int winno)
{
  PT* nibble = food->next;
  int size = food->size;
  PT* eater = hunter->next;
  int mindist=999999;
  int dist;
  int i;
  PT* tmp;

  tmp=nibble;

  for(i=0;i<size;i++) 
  {
    assert(nibble!=NULL);

    XDrawPoint(display,window[winno],color_gcs[numcolors/6],
	        nibble->x,nibble->y); 
    dist=math_dist(eater->x,eater->y, nibble->x,nibble->y);

    if(dist==0) /* Yum! */
    {
      addPoint(hunter,nibble->x,nibble->y,winno);
      XDrawPoint(display,window[winno],color_gcs[0],
	         nibble->x,nibble->y); 
      deletePoint(food,nibble);
      XFlush(display);
      return nibble;
    }
    
    if(dist<mindist)
    {
      mindist=dist;
      tmp=nibble;
    }
    nibble=nibble->next;
  }
 
  return tmp;
}

void
drawBlob(ptList *list,int color,int winno)
{
  XPoint* blobpts; 
  PT* tmp;
  int i=0;
  int size = list->size;

  blobpts = (XPoint*)malloc(sizeof(XPoint)*(size+1));
  assert(blobpts!=0);
  tmp= list->next;
  while(i<size)
  {
    assert(tmp!=NULL);
    blobpts[i].x=tmp->x;
    blobpts[i].y=tmp->y;
    i++;
    tmp=tmp->next;
  }
  blobpts[size].x=blobpts[0].x;
  blobpts[size].y=blobpts[0].y;
/*  XDrawLines(display,window[winno],color_gcs[color],
		blobpts,size+1,CoordModeOrigin); */
  XFillPolygon(display,window[winno],color_gcs[color],
		blobpts,size+1,Complex,CoordModeOrigin);
  free(blobpts);
  return;
}

void
moveBlob(int winno)
{
  int i,k;
  PT* tmp;

  for(k=0;k<options.number;k++)
  {
    PT* prey;
    int size = headlist[winno][k]->size;
 
    i=0;
    drawBlob(headlist[winno][k],0,winno);
    tmp= headlist[winno][k]->next;
    assert(tmp!=NULL);
    assert(foodlist[winno]!=NULL);
    prey=getClosestFood(headlist[winno][k],foodlist[winno],winno);
    tmp->x += sgn(prey->x-tmp->x)*2;
    tmp->y += sgn(prey->y-tmp->y)*2;
    while(i<size)
    {
      PT* checkme;
      
      tmp=tmp->next;
    /* move one cell ... move everyone else to keep the body intact */
      i++;
      tmp->x += (sgn(tmp->next->x-tmp->x) + sgn(tmp->prev->x - tmp->x));
      tmp->y += (sgn(tmp->next->y-tmp->y) + sgn(tmp->prev->y - tmp->y));

      checkme=tmp->next;
    
      do
      {
/*      fprintf(stderr,"checkme = %p checkme->next = %p checkme->prev = %p\n",
	checkme,checkme->next,checkme->prev);
*/
	if(checkme->x == tmp->x && checkme->y == tmp->y)
	{
	  tmp->x +=(rndm(5)-2);
	  tmp->y +=(rndm(5)-2);
	  checkme=tmp->next;      /* reset the checker */
	}
	else
	{
	  checkme= checkme->next;
	}
      } while(checkme->next!=tmp);

/* XDrawPoint(display,window[winno],color_gcs[tmp->pixel],tmp->x,tmp->y); */
      if(options.multi)
      {
	tmp->pixel=(tmp->pixel++)%(numcolors-1);
      }
    }
    drawBlob(headlist[winno][k],tmp->pixel,winno);
  }
  if(rndm(100)==0)
    addPoint(foodlist[winno],rndm(CX[winno]),rndm(CY[winno]),winno);
  return;
}

void
blobAddFood(int winno, int x, int y)
{
  addPoint(foodlist[winno],x,y,winno);
}

