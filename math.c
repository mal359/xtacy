#include <math.h>
#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include "lmath.h"
#include "complex.h"

static char rcsid[]="$Id: math.c,v 1.2 1994/09/17 21:51:09 white Exp $";
/*
   Complex routines and Line/Curve distance by Jer Johnson
   (mpython@gnu.ai.mit.edu)
   Fixed point math and Distance tables by Jim Finnis (white@elf.dircon.co.uk)
*/
/*
   Math routines and lookup table management
*/
      
#define	DISTSIZ	200

signed int sinTab[4096];		/*sine table*/
unsigned short wSinTab[4096];		/*0-32767 sintable, for waves*/

#if __TURBOC__
int huge distTab[DISTSIZ][DISTSIZ];
#else
static unsigned int distTab[DISTSIZ][DISTSIZ]; /* distance table */
#endif

void
math_init()
{
  long i,j;
  float f,angle_rads,pi2;

  pi2=2.0*M_PI/4096.0;

  for(i=0;i<4096;i++)
  {
    angle_rads=i*pi2;

    sinTab[i]=(signed int)(sin(angle_rads)*32767.0);
    wSinTab[i]=(unsigned short)sinTab[i]/2+16383;
  }

  /*
   *  Work out the distance table for the ripple and blur functions
   */
  
  for(i=0;i<DISTSIZ;i++)
  {
    for(j=0;j<DISTSIZ;j++)
    {
      f=(float)(i*i+j*j);
      distTab[i][j]=(unsigned int)sqrt(f);
    }
  }
}

/*
	This returns the sin of a value, with units of
	0-4095 == 0-360deg.
*/
int
mSin(int a)
{
  while(a>=4096) a-=4096;
  while(a<0) a+=4096;
  return(sinTab[a]);
}

/*
	See above for units
*/
int
mCos(int a)
{
  while(a<0)
    a+=4096;
  return(sinTab[(a+1024)%4096]);
}

/*
 *	returns the distance between two points
 */

unsigned int
math_dist(int x1,int y1,int x2,int y2)
{
  long dx,dy,mul=0L;

  dx=(long)x1-(long)x2;
  if(dx<0L)
    dx= -dx;

  dy=(long)y1-(long)y2;
  if(dy<0L)
    dy= -dy;

  /* scale down if necessary */
  while(dx>=DISTSIZ || dy>=DISTSIZ)
  {
    dx>>=1; dy>>=1;
    mul++;
  }
  return(distTab[dx][dy]<<mul);
}

/*
    Calculate the distance between a point and a line
 */
unsigned int
line_dist(int x,int y, int x1, int y1, int x2, int y2)
{
  unsigned int dist;
  unsigned int ll = math_dist(x1,y1,x2,y2);

  float r = (((y1-y) * (y1-y2)) - ((x1-x)*(x2-x1))) /(float) (ll*ll);

  if(r<0)
  {
    dist=math_dist(x,y,x1,y1);
  }
  else if (r>1)
  {
    dist=math_dist(x,y,x2,y2);
  }
  else
  {
    float s = (((y1-y) * (x2-x1)) - ((x1-x)*(y2-y1))) /(float) (ll*ll);
    dist= fabs(ll * s); 
  }

  return dist;
}

/*
     A curve is a set of lines..
 */
unsigned int
curve_dist(int x,int y, XPoint *pts, int npts)
{
  unsigned int dist=99999999;
  unsigned int i;
  for(i=0; i< npts-1; i++)
  {
    unsigned int d;
    if((d=line_dist(x,y,pts[i].x,pts[i].y,pts[i+1].x,pts[i+1].y)) <dist)
    {
       dist=d;
    }
  }

  return dist;
}



/* from the sci.fractals FAQ
Q9a: How does complex arithmetic work?
A9a: It works mostly like regular algebra with a couple additional formulas:
  (note: a,b are reals, x,y are complex, i is the square root of -1)
  Powers of i: i^2 = -1
  Addition: (a+i*b)+(c+i*d) = (a+c)+i*(b+d)
  Multiplication: (a+i*b)*(c+i*d) = a*c-b*d + i*(a*d+b*c)
  Division: (a+i*b)/(c+i*d) = (a+i*b)*(c-i*d)/(c^2+d^2)
  Exponentiation: exp(a+i*b) = exp(a)(cos(b)+i*sin(b))
  Sine: sin(x) = (exp(i*x)-exp(-i*x))/(2*i)
  Cosine: cos(x) = (exp(i*x)+exp(-i*x)/2
  Magnitude: |a+i*b| = sqrt(a^2+b^2)
  Log: log(a+i*b) = log(|a+i*b|)+i*arctan(b/a)  (Note: log is multivalued.)
  Complex powers: x^y = exp(y*log(x))
  DeMoivre's theorem: x^a = r^a * [cos(a*theta) + i * sin(a*theta)]
*/

struct COMPLEX divComplex(struct COMPLEX foo, struct COMPLEX bar)
{
  struct COMPLEX temp;
  double denom= (bar.real*bar.real)+(bar.imag*bar.imag);
  temp.real = ((foo.real*bar.real)+(foo.imag*bar.imag))/(denom);
  temp.imag = ((foo.imag*bar.real)-(foo.real*bar.imag))/(denom);
  return temp;
}

struct COMPLEX logComplex(struct COMPLEX foo)
{
  struct COMPLEX temp;
  double gaaah=fabs(foo.real+foo.imag);
  if(gaaah<=0.0)
    temp.real=-10;
  else
    temp.real = log10(gaaah);
  if (foo.real ==0)
    temp.imag = 0;
  else
    temp.imag = atan2(foo.imag,foo.real);
  return temp;
}

struct COMPLEX powComplex(struct COMPLEX foo, struct COMPLEX bar)
{
  return (expComplex(multComplex(bar,logComplex(foo))));
}

struct COMPLEX expComplex(struct COMPLEX foo)
{
  struct COMPLEX temp;
  temp.real = exp(foo.real)*(cos(foo.imag));
  temp.imag = exp(foo.real)*(sin(foo.imag));
  return temp;
}

struct COMPLEX addComplex(struct COMPLEX foo, struct COMPLEX bar)
{
  struct COMPLEX temp;
  temp.real = foo.real + bar.real;
  temp.imag = foo.imag + bar.imag;
  return temp;
}

struct COMPLEX multComplex(struct COMPLEX foo ,struct COMPLEX bar)
{
  struct COMPLEX temp;
  temp.real = (foo.real * bar.real) - (foo.imag * bar.imag);
  temp.imag = (foo.imag * bar.real) + (foo.real * bar.imag);
  return temp;
}

XPoint
PolartoRect(float radius, float angle)
{
  XPoint temp;
  long ang;

  ang = (long)(angle * 2048.0 / M_PI);

  temp.x= (int)(radius*mCos(ang))>>15;
  temp.y= (int)(radius*mSin(ang))>>15;
  return temp;
}

double
RecttoPolar(int x,int y)
{
  return (atan2(y,x));
}
