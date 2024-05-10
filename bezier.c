/*
 *  Bezier Curve code
 *  (k) 1995 Jer Johnson
 *
 */
#include <stdio.h>
#include<stdlib.h>
#include <math.h>

typedef struct _xp
{
  short x;
  short y;
} XPoint;

#define NUM_PTS 21

/*static unsigned long iBernTbl[NUM_PTS+1]; */
static const unsigned long lookup_bern(const int,const int);

  static const unsigned long   iBernTbl[] = {65536,0    ,0    ,0,
					     56188,8871 ,466  ,8,
					     47775,15925,1769 ,65,
					     40247,21307,3760 ,221,
					     33554,25165,6291 ,524,
					     27648,27648,9216 ,1024,
					     22478,28901,12386,1769,
					     17997,29073,15654,2809,
					     14155,28311,18874,4194,
					     10903,26763,21897,5971,
					     8192 ,24576,24576,8192,
					     5971 ,21897,26763,10903,
					     4194 ,18874,28311,14155,
					     2809 ,15654,29073,17997,
					     1769 ,12386,28901,22478,
					     1024 ,9216 ,27648,27648,
					     524  ,6291 ,25165,33554,
					     221  ,3760 ,21307,40247,
					     65   ,1769 ,15925,47775,
					     8    ,466  ,8871 ,56188,
					     0    ,0    ,0    ,65536};
static XPoint listP[NUM_PTS+1];

XPoint*
Bezier(XPoint* points,int npoints)
{
  int i,k;
  const int L = 3;

  for(i=0;i<NUM_PTS;i++)
  {
/* fill listP with p(i/NUM_PTS-1) */
    long x=0,y=0;
    for(k=0;k<=L;k++)
    {
      unsigned long B = lookup_bern(k,i);
      x+=(points[k].x*B);
      y+=(points[k].y*B);
    }
    listP[i].x = x>>16;
    listP[i].y = y>>16;
  }
  /* PolyLine(21,(point *)listP); */
  return listP;
}

static const unsigned long lookup_bern(const int k,const int i)
{
  return iBernTbl[i*4+k];
}

#ifdef TEST
static        double bern(int, double );

void
init_bern()
{
  int i,k;
  const int L = 3;
  
  for(i=0;i<NUM_PTS;i++)
/* fill listP with p(i/NUM_PTS-1) */
   for(k=0;k<=L;k++)
    iBernTbl[i*4+k] = (unsigned long)(65535.0* bern(k,(i/(double)(NUM_PTS-1))));

}

double
bern(int k,double t)
{
  double pow1,pow2;
  pow1 = pow((1-t),3-k);
  pow2 = pow(t,k);
/*  fprintf(stderr, "K = %d T = %f\n,",k,t);
 *  fprintf(stderr,"Pow1 = %f, Pow2 = %f \n",pow1,pow2);
 */
  if( (k==0) || (k==3))
    return pow1*pow2;
  else
    return 3.0*pow1*pow2;
}



int
main()
{
  int i;
  XPoint naffgit[4] = { {50,50},{50,100},{100,100},{100,50}};
  XPoint *foobar;
 
 /* init_bern(); */

  foobar= Bezier(naffgit,4);

  for (i=0;i<NUM_PTS;i++)
  {
    printf("Foobar[%d] = (%d,%d)\n",i,foobar[i].x,foobar[i].y);
  }

  exit (0);
}
#endif

