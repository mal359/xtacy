/*
 *  Fractal code by Jer Johnson (K)1998  (jer@gweep.net)
 *
 *  julia improvements by Daniel Burn <daniel@bizo.biz.usyd.edu.au>
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "trippy.h"
#include "lmath.h"
#include "complex.h"

extern XEvent event;

void
exit_fracrect()
{
   return;
}

void
draw_fracrect(int winno,int x1,int y1,int x2,int y2,int c,int d)
{
  int xm,ym,q;

  if(options.number>12) {
    options.number=12;
  }
  if(d>options.number)return; /* only go 12 levels deep */
  c=c%numcolors;
  if(c==0)
    c=1;
  
  q=d+1;

  xm=(x1+x2)/2;
  ym=(y1+y2)/2;

  XFillRectangle(display,window[winno],color_gcs[c],x1,y1,x2-x1,y2-y1);

  draw_fracrect(winno,x1,ym,xm,y2,c+8/d,q);
  draw_fracrect(winno,xm,y1,x2,ym,c+16/d,q);
  draw_fracrect(winno,x1,y1,xm,ym,c+32/d,q);
  draw_fracrect(winno,xm,ym,x2,y2,c+64/d,q);
  return;
}

void
exit_mandel()
{
  return;
}

void
draw_mandel(int l,int t,int r, int b, int winno )
{
  int x,y,i;
  double left,right,top,bottom;
  struct COMPLEX C;
  struct COMPLEX Z;
  int useCY = CY[winno];
  int useCX = CX[winno];
  
  top=(double)t/(useCY*.5)-1.25;
  bottom=(double)b/(useCY*.5)-.75;
  left=(double)l/(useCX*.5)-1.875;
  right=(double)r/(useCX*.5)-1.375;
  printf("Drawing Mandel from (%f,%f) to (%f,%f)\n",
	 ((right-left)*0/useCX)+left,
	 ((bottom-top)*0/useCY)+top,
	 ((right-left)*useCX/useCX)+left,
	 ((bottom-top)*useCY/useCY)+top);

  for(y=0;y<useCY;y++)
  {
    if((y%10==0))
      if (XCheckMaskEvent(display,~0L,&event)==True)
	handle_event(&event); /* check for kills every so often */

    for(x=0;x<useCX;x++)
    {
      C.real=(double)((right-left)*x/useCX)+left;
      C.imag=(double)((bottom-top)*y/useCY)+top;
      Z.real=C.real;
      Z.imag=C.imag;	 
      for (i=1;i<(options.number*10);i++)      
      {
	Z=multComplex(Z,Z);
	Z=addComplex(Z,C);
	if((Z.real*Z.real)+(Z.imag*Z.imag)>4.0)
	  break;
      }
      XDrawPoint(display,window[winno],
		 color_gcs[(i%numcolors)?(i%numcolors):1],x,y);
    }
  }
}

void
exit_julia()
{
  return;
}

/* julia.. the fractal set, not the person */
void
draw_julia(int winno, double coordx, double coordy)
{
  int x,y,i;
  struct COMPLEX Z,Z2;
  double useCX = CX[winno]*.5;
  double useCY = CY[winno]*.4;
  
  for(y=0;y<CY[winno];y++)
  {
    if (XCheckMaskEvent(display,~0L,&event)==True)  /* check with the */
      handle_event(&event);                        /* outside world  */
    for(x=0;x<CX[winno];x++)
    {
      Z.real=(double)(x-useCX)/useCY;
      Z.imag=(double)(y-useCX)/useCY;

 /*   C.real =  -0.27334; 
      C.imag = 0.00742; */

      /* F(z)=z^2+c */
      for (i=0;i<(options.number*10);i++)
      {
	Z2.real = Z.real * Z.real - Z.imag * Z.imag + coordx;
	Z.imag = 2.0 * Z.real * Z.imag + coordy;
	Z.real = Z2.real;
	/*
	  Z=multComplex(Z,Z);
	  Z=addComplex(Z,C);
	 */
	if((Z.real*Z.real)+(Z.imag*Z.imag)>4.0)
	  break;
      }
      XDrawPoint(display,window[winno],color_gcs[(i%(numcolors-1))+1],x,y);
    }
  }
}

void
exit_newton()
{
  return;
}

/* newton.. the fractal set, not the person */
void
draw_newton(int winno)
{
  int x,y,i,D;
  struct COMPLEX Z;
  const struct COMPLEX minus1= {-1,0};
  struct COMPLEX expon;
  struct COMPLEX expDown;
  int useCX = CX[winno];
  int useCY = CY[winno];
  int midCX = useCX>>1;
  int midCY = useCY>>1;
  int val=options.number;
 
  expon.imag=expDown.imag=0;
  expon.real=val; expDown.real= val-1;

  for(D=16;D>=1;D>>=1)
  {
    for(y=0;y<useCY;y+=D)
    {
      if (XCheckMaskEvent(display,~0L,&event)==True)
	handle_event(&event);
      for(x=0;x<useCX;x+=D)
      {
	Z.real=(double)(x-midCX)/(double)midCX;
	Z.imag=(double)(y-midCY)/(double)midCY;
	/* z^val-1 = 0 */
	for (i=0;i<(options.number*10);i++)
	{
	  double lastR=Z.real; double lastI=Z.imag;
/*      Z(j+1)=Z(i)-( (Z(i)^3-1)/(3*Z(i)^2)) */
/* Whoa! this is REAL MESSY! */
	  Z= addComplex(Z,multComplex(minus1,(divComplex(
	    (addComplex(powComplex(Z,expon),minus1)),
	    (multComplex(expon,powComplex(Z,expDown)))))));

	  if((fabs(Z.real-1.0)<0.01)&&(fabs(Z.imag)<0.01))
	  {
	    /*  printf("Breaking out "); */
	    break;
	  }
/*
  fprintf(stderr,"Iter[%d] Z.real = %lf \t Z.imag= %lf\n",
  i,Z.real,Z.imag);
  */
	  if(Z.real==lastR && Z.imag==lastI) /* we're stuck.. quit now */
	  {
	    i=10*options.number;
	    break;
	  }
	}
	XFillRectangle(display,window[winno],color_gcs[(i%numcolors)+1],x,y,D,D);
/* XDrawPoint(display,window[winno],color_gcs[(i%numcolors)+1],x,y); */
      }
    }
  }
}
