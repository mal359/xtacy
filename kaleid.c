#include <X11/X.h>
#include <X11/Xlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "lmath.h"
#include "trippy.h"

typedef enum {yes,no,maybe,lizard_spit} yesno;

/* Yow! have some global pointers */
static int *X1, *Y1, *X2, *Y2, *X3, *Y3,
	   *XV1, *YV1, *XV2, *YV2, *XV3, *YV3;
static int *XA, *YA, *XB, *YB, *XC, *YC,
	   *XD, *YD, *XE, *YE, *XF, *YF;

static void boing(int);
static int inited=0;
static yesno rect;

#define midX (CX[winno]>>1)
#define midY (CY[winno]>>1)

int
init_kaleid(int nwin)
{
  int i;
  if(!inited)
  {
    fprintf(stderr,"Init'ing Kaleid\n");
    
    if ((X1 =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((Y1 =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((X2 =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((Y2 =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((X3 =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((Y3 =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((XV1=(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((YV1=(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((XV2=(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((YV2=(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((XV3=(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((YV3=(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((XA =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((YA =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((XB =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((YB =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((XC =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((YC =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((XD =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((YD =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((XE =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((YE =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((XF =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    if ((YF =(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
    rect=no;
    inited=1;
  }
  for(i=0;i<nwin;i++)
  { 
    HC[i]=rndm(numcolors);
  }
  return 1;
}

void
exit_kaleid()
{
  fprintf(stderr,"Free Kaleid\n"); 
  free(X1);
  free(Y1);
  free(X2);
  free(Y2);
  free(X3);
  free(Y3);
  free(XV1);
  free(YV1);
  free(XV2);
  free(YV2);
  free(XV3);
  free(YV3);
  free(XA );
  free(YA );
  free(XB );
  free(YB );
  free(XC );
  free(YC );
  free(XD );
  free(YD );
  free(XE );
  free(YE );
  free(XF );
  free(YF );
  inited=0;
}


void
randomize_kal(int i)
{
  X1[i] = rndm((long)M[i]) + 1;
  X2[i] = rndm((long)M[i]) + 1;
  X3[i] = rndm((long)M[i]) + 1;
  Y1[i] = rndm((long)X1[i]);
  Y2[i] = rndm((long)X2[i]);
  Y3[i] = rndm((long)X3[i]);
  
  XV1[i] = rndm(7L)-3;
  XV2[i] = rndm(7L)-3;
  XV3[i] = rndm(7L)-3;
  YV1[i] = rndm(7L)-3;
  YV2[i] = rndm(7L)-3;
  YV3[i] = rndm(7L)-3;
}

void
fill_tris(int winno)
{

  XPoint *one, *two, *three;
  int i;
  
  one   = mirror(winno,XA[winno],YB[winno], options.mirrors, 1);
  two   = mirror(winno,XC[winno],YD[winno], options.mirrors, 1);
  three = mirror(winno,XE[winno],YF[winno], options.mirrors, 1);

  for(i=0; i<options.mirrors+1; i++)
  {
    XFillPolygon(display,window[winno],color_gcs[HC[winno]],
		 make_tri(one[i].x,one[i].y,
			  two[i].x,two[i].y,
			  three[i].x, three[i].y),
		 3,Convex,CoordModeOrigin);
  }
  free(one);
  free(two);
  free(three);
}

void
fill_rects(int winno)
{
  XPoint *one, *two;
  int i;
  
  one   = mirror(winno,XA[winno],YB[winno], options.mirrors, 1);
  two   = mirror(winno,XC[winno],YD[winno], options.mirrors, 1);

  for(i=0; i<options.mirrors+1; i++)
  {
    XFillPolygon(display,window[winno],color_gcs[HC[winno]],
		 wrecked(one[i].x,one[i].y,
			 two[i].x,two[i].y),
		 4,Convex,CoordModeOrigin);
  }
  free(one);
  free(two);

}

void
fill_circles(int winno)
{ 
  XPoint *one, *two;
  int i;
  
  one   = mirror(winno,XA[winno],YB[winno], options.mirrors, 1);
  two   = mirror(winno,XC[winno],YD[winno], options.mirrors, 1);

  for(i=0; i<options.mirrors+1; i++)
  {
    XFillPolygon(display,window[winno],color_gcs[HC[winno]],
		 make_circle(one[i].x, one[i].y,
			     two[i].x, two[i].y),
		 33,Nonconvex,CoordModeOrigin);
  }
  free(one);
  free(two);
}

void
draw_lines(int winno)
{
  XPoint *one, *two;
  int i;
  
  one   = mirror(winno,XA[winno],YB[winno], options.mirrors, 1);
  two   = mirror(winno,XC[winno],YD[winno], options.mirrors, 1);

  for(i=0; i<options.mirrors+1; i++)
  {
    XDrawLine(display,window[winno],color_gcs[HC[winno]],
	      one[i].x, one[i].y,
	      two[i].x, two[i].y);
  }
  free(one);
  free(two);  
}

XPoint *
wrecked(int x1, int y1, int x2, int y2)
{
  static XPoint temp[4];
  
  temp[0].x = x1; temp[0].y = y1;
  temp[1].x = x2; temp[1].y = y1;
  temp[2].x = x2; temp[2].y = y2;
  temp[3].x = x1; temp[3].y = y2;
  
  return temp;
}

XPoint *
make_tri(int x1,int y1, int x2, int y2, int x3, int y3)
{
  static XPoint temp[3];

  temp[0].x = x1; temp[0].y = y1;
  temp[1].x = x2; temp[1].y = y2;
  temp[2].x = x3; temp[2].y = y3;
  
  return temp;
}



XPoint *
make_circle(int x1,int y1,int x2,int y2)
{
  /* (x1,y1) is the center;
  ** (x2,y2) is a point on the circle;
   */
  int radius;
  int count;
  int addx,addy;
  static XPoint temp[33];
  
  radius = math_dist(x1,y1,x2,y2);
  
  for(count=0;count<=8;count++)
  { 
/* Finally added sin and cos, instead of faking it with Pythag's Theorem */
/* and added a Sin and Cos table, instead of calling the functions every
   bloody time */
    addx=(long)(radius*mSin(count<<7))>>15;
    addy=(long)(radius*mCos(count<<7))>>15;
    temp[count].x=x1+addx;       temp[count].y=y1+addy;
    temp[16-count].x= x1+addx;   temp[16-count].y= y1-addy;
    temp[count+16].x= x1-addx;   temp[count+16].y= y1-addy;
    temp[32-count].x= x1-addx;   temp[32-count].y= y1+addy;
  }
  temp[32].x=temp[0].x;  temp[32].y=temp[0].y;
  
  return temp;  
}

void
draw_kaleid(int winno)
{ 
  /*  fprintf(stderr,"CX[winno]=%d CY[winno]=%d M[winno]=%d\n",
      CX[winno],CY[winno],M[winno]);
  */
  if (((!rndm(15L)&&(rect==lizard_spit)))||(!rndm(40L))) 
  {
    switch (rect)
    {
      case yes:
/*	rect=lizard_spit; break; */
      case no:
	rect=maybe; break;
      case maybe:
/*	rect=yes; break; */
      case lizard_spit:
	rect=no; break;
    }
  }
  boing(winno);

  if (options.norect) 
  {
    draw_lines(winno);
  }
  else
  {
    switch (rect)
    {
      case no:
      {
	draw_lines(winno);
	break;
      }
      /* case yes:
      {
	fill_rects(winno);
	break;
      }
      */
      case maybe:
      { 
	fill_tris(winno);
	break;
      }
      /*
      case lizard_spit:
      {
	fill_circles(winno);
	break;
      }
      */
    }
  }
}


void
boing(int winno)
{
  if (!rndm(50L)) 
  {
    X1[winno] = rndm(M[winno]) + 1;
    X2[winno] = rndm(M[winno]) + 1;
    X3[winno] = rndm(M[winno]) + 1;
    Y1[winno] = rndm(M[winno]);
    Y2[winno] = rndm(M[winno]);
    Y3[winno] = rndm(M[winno]);
  }
  
  if (!rndm(10L)) 
  {
    XV1[winno] = rndm(7L)-3;
    XV2[winno] = rndm(7L)-3;
    XV3[winno] = rndm(7L)-3;
    YV1[winno] = rndm(7L)-3;
    YV2[winno] = rndm(7L)-3;
    YV3[winno] = rndm(7L)-3;
    if (options.multi)
      HC[winno] = (HC[winno]+1)%numcolors;
    else
      HC[winno] = rndm((long)numcolors);
  }
  
  XA[winno] = X1[winno];
  YA[winno] = Y1[winno];
  XB[winno] = (long)X1[winno];
  YB[winno] = (long)Y1[winno];
  XC[winno] = X2[winno];
  YC[winno] = Y2[winno];
  XD[winno] = (long)X2[winno];
  YD[winno] = (long)Y2[winno];
  XE[winno] = X3[winno];
  YE[winno] = Y3[winno];
  XF[winno] = (long)X3[winno];
  YF[winno] = (long)Y3[winno];
  
  X1[winno]= (X1[winno] + XV1[winno]) % M[winno];
  Y1[winno]= (Y1[winno] + YV1[winno]) % M[winno];
  X2[winno]= (X2[winno] + XV2[winno]) % M[winno];
  Y2[winno]= (Y2[winno] + YV2[winno]) % M[winno];
  X3[winno]= (X3[winno] + XV3[winno]) % M[winno];
  Y3[winno]= (Y3[winno] + YV3[winno]) % M[winno];
}
