/*
 *   Lightning Strike, written by jer johnson (mpython@gnu.ai.mit.edu |
 *                                             jer@sidehack.gweep.net |
 *                                             jer@ultranet.com)
 *   on 15 Oct 1995
 *   (c) 1995 jer johnson under GNU License v2
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "trippy.h"

#define MAXTINES 400

struct zap 
{
  int x,y,num;
  struct zap *left, *right;
  struct zap *next;
};

static int init=0;
struct zap ***zaparray;

void
exit_lightning()
{
  int i,j;
  
  for(j=0;j<options.windows;j++)
  {
    for(i=0;i<MAXTINES;i++)
      free(zaparray[j][i]);
    free(zaparray[j]);
  }
  free(zaparray);
  init=0;
}

void
lightning_strike(int winno)
{
/* hmm.. draw lines ... change directions and/or fork from time to time */
  struct zap *head, *ptr;
  int clr= rndm(numcolors);
  int num=0;
  int servicenum=0;


  if(!init) {
    int i,j;

    zaparray = (struct zap ***)malloc(sizeof(struct zap **)*options.windows);
    for(j=0;j<options.windows;j++)
    {
      zaparray[j] = (struct zap **)malloc(MAXTINES*sizeof(struct zap*)); 
      for(i=0;i<MAXTINES;i++)
	zaparray[j][i]=(struct zap*)malloc(sizeof(struct zap));
    }
    init=1;
  }

  head = zaparray[winno][0]; 
  head->x=rndm(CX[winno]);
  head->y=0;
  head->num=0;
  head->left = head->right = head->next = 0;
  ptr = head;
  num++;
  do
  {
    if(num>=MAXTINES)
	break;
    ptr->left=zaparray[winno][num];
    ptr->left->x=ptr->x+(rndm(10)+rndm(10))-9;
    ptr->left->y=ptr->y+(rndm(10));
    ptr->left->num=num++;
    
    if(ptr->next==NULL)
      ptr->next=ptr->left;
    
    if(!rndm(50)) /* then fork */
    {
      if(num>=MAXTINES)
        break;
      ptr->right= zaparray[winno][num];
      ptr->right->x=ptr->x+(rndm(10)+rndm(10))-9;
      ptr->right->y=ptr->y+(rndm(10));
      ptr->right->num=num++;
      ptr->left->next = ptr->right;
      ptr->right->next=NULL;
    }
    else
      ptr->right=NULL;
    
/* now draw */
    if(ptr->left!=NULL)
    {
/*      fprintf(stderr," %d -> %d Left\n",ptr->num,ptr->left->num); */
      XDrawLine(display,window[winno],color_gcs[clr],ptr->x,ptr->y,
		ptr->left->x,ptr->left->y);
   }
    if(ptr->right!=NULL)
    {
/*      fprintf(stderr," %d -> %d Right\n",ptr->num,ptr->right->num); */
      XDrawLine(display,window[winno],color_gcs[clr],ptr->x,ptr->y,
		ptr->right->x,ptr->right->y);
    }
    if(options.multi)
      clr= (clr+1)%numcolors;
/* and continue down the chain */
    if(servicenum>=MAXTINES-1)
    {
      break;
    }
    ptr=zaparray[winno][++servicenum];
  } while(ptr->y<CY[winno] && servicenum<MAXTINES);
  ptr=0;
}

void
de_strike(int winno)
{
  int servicenum=0;
  struct zap *ptr=zaparray[winno][servicenum];
  while(ptr!=NULL)
  {
    if(servicenum > MAXTINES) break;

    if(ptr->left!=NULL)
      XDrawLine(display,window[winno],color_gcs[0],ptr->x,ptr->y,
		ptr->left->x,ptr->left->y);
    if(ptr->right!=NULL)
      XDrawLine(display,window[winno],color_gcs[0],ptr->x,ptr->y,
		ptr->right->x,ptr->right->y);

    ptr->right = NULL;
    ptr->left  = NULL;
    ptr->next  = NULL;

    servicenum++;
    ptr=zaparray[winno][servicenum];
  }
}

