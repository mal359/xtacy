/*
 *	Wave stuff
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include "trippy.h"
#include "lmath.h"
#include "waves.h"
#include <stdio.h>
#include <stdlib.h>

struct wave
{
  long size;
  unsigned short *vals;
};

static struct wave Waves[MAXWAVES];
static long wavct=0;

unsigned short*
get_wave(int ind)
{
  return Waves[ind].vals;
}

long
wave_new(long size)
{
  if(wavct==MAXWAVES)fprintf(stderr,"too many waves");

  if(!(Waves[wavct].vals=(unsigned short *)calloc(Waves[wavct].size=size,
						  sizeof(short))))
    fprintf(stderr,"not enough memory for wave");

  return(wavct++);
}

long
wave_new_table(long size,unsigned short *tab)
{
  if(wavct==MAXWAVES)fprintf(stderr,"too many waves");

  Waves[wavct].size=size;
  Waves[wavct].vals=tab;

  return(wavct++);
}

void
wave_set(long wavindex,long index,short val)
{
  if(wavindex>=wavct)fprintf(stderr,"bad wave in set %ld",wavindex);
  Waves[wavindex].vals[index]=(short)val;
}

void
wave_drawh(int winno,long wavindex,long x,long y,long len,long offset,
	   long scale,long intensity,long paloffset)
{
  unsigned long size,i,palsize,start,index;
  unsigned long col;
  unsigned short *data;

  palsize=numcolors;

  if(wavindex>=wavct)fprintf(stderr,"bad wave in draw %ld",wavindex);

  if(y>CY[winno])return;
  if(x+len > CX[winno])len=CX[winno]-x;

  data=Waves[wavindex].vals;
  if(!(size=Waves[wavindex].size))fprintf(stderr,"funny wave %ld",wavindex);

  /* work out the start */

  start=(offset*size)/100L;

  for(i=0;i<len;i++)
  {
/*  graph_check(); */
    /* get index into table */
    index=i*size*scale;
    index=index/(len*100L)+start;
    index %= size;

    col=data[index]*intensity; /* data in the wave is 0-32767 */
    /* convert to a colour */
    col/=100L*(32768L/palsize);
    /* add the palette offset and take the mod */
    col=(col+paloffset)%palsize;
      
    /* plot! */
/*      buffer_point(x++,y,col); */
    XDrawPoint(display,window[winno],color_gcs[col],x++,y);
  }
}
	  
void
wave_drawv(int winno,long wavindex,long x,long y,long len,long offset,
	   long scale,long intensity,long paloffset)
{
  unsigned long size,i,palsize,start,index;
  unsigned long col;
  unsigned short *data;

  palsize=numcolors;

  if(wavindex>=wavct)
    fprintf(stderr,"bad wave in draw %ld",wavindex);

  if(x>CX[winno])return;
  if(y+len > CY[winno])len=CY[winno]-y;

  data=Waves[wavindex].vals;
  if(!(size=Waves[wavindex].size))
    fprintf(stderr,"funny wave %ld",wavindex);

  /* work out the start */

  start=(offset*size)/100L;

  for(i=0;i<len;i++)
  {
/*      graph_check(); */
      /* get index into table */
    index=i*size*scale;
    index=index/(len*100L)+start;
    index %= size;
      
    col=data[index]*intensity; /* data in the wave is 0-32767 */
    /* convert to a colour */
    col/=100L*(32768L/palsize);
    /* add the palette offset and take the mod */
    col=(col+paloffset)%palsize;
	  
    /* plot! */
/*	  buffer_point(x,y++,col); */
    XDrawPoint(display,window[winno],color_gcs[col],x,y++);
  }
}

/*
 *   Set up the internal waves
 */

static long SawWave,SineWave;
void
wave_init()
{
  long i;

  extern unsigned short wSinTab[];	/* from math.h */
  SineWave=wave_new_table(4096,wSinTab);
  SawWave=wave_new(4096);

  for(i=0;i<2048;i++)
    wave_set(SawWave,i,i*(32767/2048));
  for(i=2048;i<4096;i++)
    wave_set(SawWave,i,(4096-i)*(32767/2048));
}

void
wave_radial(int winno,long handle,long x,long y,
	    long size,long offset,long scale,long intensity)
{
  unsigned short *data;
  long palsize,nom,denom,wavsize,cdenom;
  long xp,yp,xs,ys;
  register long cx,cy,dist,col;
  
  palsize=numcolors;

  if(handle>=wavct)
    fprintf(stderr,"bad wave in radial %ld",handle);
  
  wavsize=Waves[handle].size;
  data=Waves[handle].vals;
	
  /* work out ratios */

  nom=wavsize*scale;
  denom=size*100L;

  cdenom=100L*(32768L/palsize);
  
  xs=x+size;ys=y+size;
  xp=x-size;yp=y-size;
	
  if(xs>CX[winno])xs=CX[winno];
  if(ys>CY[winno])ys=CY[winno];
  if(xp<0)xp=0;
  if(yp<0)yp=0;

  offset *=wavsize;
  offset /=100;
	
  /* and looooooop */

  for(cx=xp;cx<xs;cx++)
  {
    for(cy=yp;cy<ys;cy++)
    {
      dist=math_dist(cx,cy,x,y);

      dist*=nom;
      dist/=denom;
	      
      col=(long)(data[(dist+offset)%wavsize]) * intensity;
      /* buffer_point(cx,cy,col/cdenom); */
      XDrawPoint(display,window[winno],color_gcs[col/cdenom],cx,cy);
    }
  }
}

void
exit_waves()
{
  return;
}

void
exit_radial()
{
  return;
}
