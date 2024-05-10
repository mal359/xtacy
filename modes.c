/*
 *   Other modes
 *
 *   mixer, cells modes and improved Spiral by Daniel Cabeza Gras
 *                                        ( bardo@clip.dia.fi.upm.es )
 *
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "trippy.h"
#include "lmath.h"
#include "waves.h"

extern XEvent event;

void
exit_spiral()
{
  return;
}

void
draw_spiral(int winno)
{     
/* A spiral has the formula rad=ang */

  XPoint plotme;
  float ang=0.0;
  short clr=rndm(numcolors-1);
  int theend= math_dist(0,0,CX[winno],CY[winno]);
  float fact =(float)options.number/3;
  int useCX= CX[winno]>>1;
  int useCY= CY[winno]>>1;
  
  do
  {
    plotme=PolartoRect(ang*fact,ang);
    
/*  fprintf(stderr,"ang= %f plotme = (%d,%d)\n",ang,plotme.x, plotme.y); */
    
    ang+=(float)M_PI/737;
    if((abs(plotme.x)<(useCX))&&(abs(plotme.y)<(useCY)))
      XDrawPoint(display,window[winno],color_gcs[clr],
		 plotme.x+useCX,plotme.y+useCY);
     
    clr--;
    if(clr<=0)
      clr=numcolors-1;
  } while((((ang*fact)*.66667)) <= theend);
}

void
exit_wandering()
{
  return;
}

void
wander(int winno,int *x, int *y)
{
  int spoo;
 
  XDrawPoint(display,window[winno],color_gcs[HC[winno]],*x,*y);
  if((spoo=rndm(3))==1)
    *x+=(rndm(3));
  else if (spoo==2)
    *x-=(rndm(3));
  if((spoo=rndm(3))==1)
    *y+=(rndm(3));
  else if (spoo==2)
    *y-=(rndm(3));
  if (*x<=0)
    *x=CX[winno];
  else if (*x>=CX[winno])
    *x=0;
  if (*y<=0)
    *y=CY[winno];
  else if (*y>=CY[winno])
    *y=0;

  return;
}

void
exit_funky()
{
  return;
}

void
draw_funky(int winno)
{
  static float lengthmod= 0.1;
  static float length=1;
  static float ang=0.0;
  static short clr=0;
  XPoint plotme,plotme2;
  int halfX=CX[winno]>>1;
  int halfY=CY[winno]>>1;
 
  int howfunky=options.number; /* just HOW funky do you want it? */

  length += lengthmod;
  if (length <= 0.0 || length >= M[winno]) {
    lengthmod *= -1;
  }
  plotme=PolartoRect(length,ang);

  if(howfunky==1)
  {
    XPoint eris[5]; /* -><- Hail Eris! -><- */
    plotme2=PolartoRect(length,ang+M_PI_2);
    eris[0].x=eris[4].x=plotme.x+(halfX);
    eris[0].y=eris[4].y=plotme.y+(halfY);
    eris[1].x=plotme2.x+(halfX);
    eris[1].y=plotme2.y+(halfY);
    eris[2].x=(halfX)-plotme.x;
    eris[2].y=(halfY)-plotme.y;
    eris[3].x=(halfX)-plotme2.x;
    eris[3].y=(halfY)-plotme2.y;
    XDrawLines(display,window[winno],color_gcs[clr],eris,5,
	       CoordModeOrigin);
  }
  else if(howfunky==2)
  {
    XPoint plotme3;
    XPoint eris[4];
    plotme2 = PolartoRect(length,ang+(.66667*M_PI));
    plotme3 = PolartoRect(length,ang+(1.3333*M_PI));
    eris[0].x=eris[3].x=plotme.x+(CX[winno]>>1);
    eris[0].y=eris[3].y=plotme.y+(CY[winno]>>1);
    eris[1].x=plotme2.x+(CX[winno]>>1);
    eris[1].y=plotme2.y+(CY[winno]>>1);
    eris[2].x=plotme3.x+(CX[winno]>>1);
    eris[2].y=plotme3.y+(CY[winno]>>1);
    XDrawLines(display,window[winno],color_gcs[clr],eris,4,
		CoordModeOrigin);
  }
  else
  {
    XDrawLine(display,window[winno],color_gcs[clr],
		plotme.x+(CX[winno]/2),plotme.y+(CY[winno]/2),
		(CX[winno]/2)-plotme.x,(CY[winno]/2)-plotme.y);
  }
  ang+=(float)(M_PI/737);
  clr--;
  if(clr<=0)
    clr=numcolors-1;
  return;
}

/* 
 *  Ripple in still water
 *  Where there is no pebble tossed
 *  Nor wind to blow
 *         - R. Hunter
 */

void
draw_ripple(int winno)
{
  int clr;
  int a;
  XGCValues gcval;
 
  if(options.opt1%2)
  { 
    int x,y;
    register long dist;
    int *centerx=calloc(options.number,sizeof(int));
    int *centery=calloc(options.number,sizeof(int));
    int *colors=calloc(options.number,sizeof(int));
    for(a=0;a<options.number;a++)
    {
      centerx[a]=rndm(CX[winno]);
      centery[a]=rndm(CY[winno]);
      colors[a]= rndm(numcolors);
    }
    for(x=0;x<CX[winno];x++)
      for(y=0;y<CY[winno];y++)
      {
        int mindist=999999;
        for(a=0;a<options.number;a++)
        {
	  if (((dist=math_dist(centerx[a],centery[a],x,y)))<mindist)
	  {
	    mindist=dist;
	  }
        }
	/* clr /= options.number; */
	clr = mindist%(numcolors-1);
        XDrawPoint(display,window[winno],color_gcs[clr],x,y);
      }

  }
  else
  {
    wave_radial(winno,0,rndm(CX[winno]),rndm(CY[winno]),M[winno],0,25,50);
    for(clr=1;clr<numcolors;clr++)
    {
      gcval.function=GXxor;
      XChangeGC(display,color_gcs[clr],GCFunction,&gcval);
    }

    for(a=0;a<(options.number*2);a++)
    {
      wave_radial(winno,0,rndm(CX[winno]),rndm(CY[winno]),M[winno],0,25,50);
    }
  }
  return;
}

void
exit_ripple()
{
  int clr;
  XGCValues gcval;

  for(clr=1;clr<numcolors;clr++)
  {
    gcval.function=GXcopy;
    XChangeGC(display,color_gcs[clr],GCFunction,&gcval);
  }
}

void
exit_xstatic()
{
  return;
}

void
draw_static(int winno)
{
  int x,y;

  randomize_colors();
  for(y=0;y<=CY[winno]>>1;y++)
    for(x=0;x<=CX[winno];x++)
    {
      int r=rndm(numcolors-1)+1;
      XDrawPoint(display,window[winno],color_gcs[r],x,y);
      XDrawPoint(display,window[winno],color_gcs[r],CX[winno]-x,CY[winno]-y);
    }
}


void
exit_mixer()
{
  return;
}

/* The paint mixer, by //)aniel Cabeza Gras (bardo@clip.dia.fi.upm.es) */
void
draw_mixer(int winno)
{
  int x,y,X,Y,Color,HYP,fact;
  int useCX = rndm(CX[winno]);
  int useCY = rndm(CY[winno]);
  
  HYP = (useCX*useCX+useCY*useCY)/4;
  fact = options.number * 4;
  
  for(y=0;y<CY[winno];y++)
  {
    Y=y-(useCY);
    if (XCheckMaskEvent(display,~0L,&event)==True)/* check the outside world */
      handle_event(&event);
    for(x=0;x<CX[winno];x++)
    {
      X=x-useCX;
      Color=((X==0)&&(Y==0)) ? 0 :
	(((int)floor((((atan2((double)Y,(double)X)/M_PI)+1)/2+
		      (1+cos(M_PI*(double)(X*X+Y*Y)/HYP*fact))/(fact>>1))*
		     (numcolors-1)))%(numcolors-1))+1;
      XDrawPoint(display,window[winno],color_gcs[Color],x,y);
    }
  }
}

void
exit_taffy()
{
  return;
}

/* taffy maker, by Steve Richardson (prefect@sidehack.gweep.net)
 * based off of paint mixer
 */
void
draw_taffymaker(int winno)
{
  int x,y,X,Y,Color,HYP,fact;
  fact = options.number * 2;
  
  HYP = (CX[winno]*CX[winno]+CY[winno]*CY[winno])/4;
  for(y=0;y<CY[winno];y++)
  {
    Y=y-(CY[winno]/2);
    if (XCheckMaskEvent(display,~0L,&event)==True)
      handle_event(&event);
    for(x=0;x<CX[winno];x++)
    {
      X=x-(CX[winno]/2);
      Color=((X==0)&&(Y==0)) ? 0 :
	(((int)floor((((atan2((double)X,(double)Y)/M_PI)+1)/2+
		      (1+sin(M_PI*(double)(X*X-Y*Y)/HYP*fact))/fact)*
		     (numcolors-1)))%(numcolors-1))+1;

      XDrawPoint(display,window[winno],color_gcs[Color],x,y);
    }
  }
}

void
exit_cells()
{
  return;
}

/* The cells by //)aniel Cabeza Gras (bardo@clip.dia.fi.upm.es) */
void
draw_cells(int winno)
{
  int x,y,Color;
  double fact = (double) options.number / (double) CY[winno];
  
  for(y=0;y<CY[winno];y++)
  {
    if (XCheckMaskEvent(display,~0L,&event)==True)
      handle_event(&event);
    for(x=0;x<CX[winno];x++)
    {
      Color=(((int)floor((sin(M_PI*(double) x*fact)*
			  sin(M_PI*(double) y*fact) + 1)/2*
			 (numcolors-1))) %(numcolors-1))+1;

      XDrawPoint(display,window[winno],color_gcs[Color],x,y);
    }
  }
}

void
exit_dcurve()
{
  return;
}

void
draw_dcurve(int winno)
{
  int i,x,y;
  
  XPoint** bezi;

  bezi = (XPoint**)malloc(options.number*sizeof(XPoint*));

  for(i=0;i<options.number;i++)
  {
    XPoint makeme[4];
    int j;
   
    for(j=0;j<4;j++)
    {
      makeme[j].x = rndm(CX[winno]);
      makeme[j].y = rndm(CY[winno]);      
    }
  
    bezi[i] = (XPoint*)malloc(sizeof(XPoint)*21);
    memcpy(bezi[i], Bezier(makeme,21), sizeof(XPoint)*21);
  }
  
  for(x=0;x<CX[winno];x++)
    for(y=0;y<CY[winno];y++)
    {
      unsigned int d = 999999;
      for(i=0;i<options.number;i++)
      {
        unsigned int dp;
        if((dp=curve_dist(x,y,bezi[i],21))<d)
	  d=dp;
      }
      if(d==0) d=1;
      XDrawPoint(display,window[winno],color_gcs[d%numcolors],x,y);
    }

  for(i=0;i<options.number;i++)
  {
    free(bezi[i]);
  }
  free(bezi);
}

void
exit_dline()
{
  return;
}

void
draw_dline(int winno)
{
  int *x1,*x2,*y1,*y2;
  int i,x,y;  

  x1=malloc(options.number * sizeof(int));
  y1=malloc(options.number * sizeof(int));
  x2=malloc(options.number * sizeof(int));
  y2=malloc(options.number * sizeof(int));
  /* first, we draw the line */
  for(i=0;i<options.number;i++)
  {
    x1[i]=rndm(CX[winno]); x2[i]=rndm(CX[winno]);
    y1[i]=rndm(CY[winno]); y2[i]=rndm(CY[winno]);
    XDrawLine(display,window[winno],color_gcs[rndm(numcolors)],
	      x1[i],y1[i],x2[i],y2[i]);
  }
  for(x=0;x<CX[winno];x++)
    for(y=0;y<CY[winno];y++)
    {
      unsigned int d=999999;
      for (i=0;i<options.number;i++)
      {
        unsigned int dp;
        if((dp=line_dist(x,y,x1[i],y1[i],x2[i],y2[i]))<d)
	  d=dp;
      }
      if(d==0) d=1;
      XDrawPoint(display,window[winno],color_gcs[d%numcolors],x,y);
    }
}

void
exit_sunrise()
{
  return;
}

void
draw_sunrise(int winno)
{
  int x,y,dx,dy,color,dist,angle;
  int xcenter= CX[winno]>>1;
  int ycenter= CY[winno]>>1; 
  
  angle=0; dist=0;
  
  for(y=0;y<CY[winno];y++)
  {
    if (XCheckMaskEvent(display,~0L,&event)==True)
      handle_event(&event);
    for(x=0;x<CX[winno];x++)
    {
      color = (angle % 64) ^ dist;
      dx = x - xcenter;
      dy = y - ycenter;
      dist = math_dist(0,0,dx, dy);
      angle = (int)(RecttoPolar(dx, dy)*options.number);
      do
      {
/*	color = color%(numcolors-1); */
	if(color<=0)
	  color+=(numcolors-1);
	else if (color>=numcolors)
	  color-=numcolors;
      } while (color<=0 || color>=numcolors);

      XDrawPoint(display,window[winno],color_gcs[color],x,y);
    }
  }
  return;
}


void
exit_tunnel()
{
  return;
}

void
draw_tunnel(int winno)
{
  int i,rev;
  XPoint points[4];
  int npoints = 4, shape = Convex,  mode = CoordModeOrigin;
  float frac;
  int min_x, max_x, mid_x, min_y, max_y, mid_y;
  int useCX = CX[winno] / (options.mirrors+1);
  int useCY = CX[winno] / (options.mirrors+1);
  
  rev=rndm(2);
  
  min_x = 0 - useCX/2;
  max_x = useCX * 3 / 2;
  mid_x = useCX / 2;
  min_y = 0 - useCY/2;
  max_y = useCY * 3 / 2;
  mid_y = useCY / 2;
  points[0].y = points[2].y = (short) mid_y;
  points[1].x = points[3].x = (short) mid_x;
  for (i=0; i<numcolors; i++)
  {
    frac = (float) i/numcolors;
    points[0].x = (short) (min_x + useCX * frac);
    points[2].x = max_x - points[0].x + min_x;
    points[1].y = (short) (min_y + useCY * frac);
    points[3].y = max_y - points[1].y + min_y;
    if(rev)
      XFillPolygon (display, window[winno], color_gcs[numcolors-i-1], points,
		    npoints, shape, mode);
    else
      XFillPolygon (display, window[winno], color_gcs[i], points,
		    npoints, shape, mode);
  }
  if(options.mirrors)
  {
    int x,y;
    for (x=0;x<=options.mirrors;x++)
      for (y=0;y<=options.mirrors;y++)
      {
	XCopyArea(display,window[winno],window[winno],color_gcs[1],
		  0,0,useCX,useCY,useCX*x,useCY*y);
      } 
  }
}

void
exit_clover()
{
  return;
}

void
draw_clover(int winno)
{
  int x,y,gaaak;
  int useCX = CX[winno] / (options.mirrors+1);
  int useCY = CY[winno] / (options.mirrors+1);
  int midX=useCX>>1;
  int midY=useCY>>1;
  int col=1; float colstep=1;

  int numLines=2*useCX+2*useCY;
  int numberToSkip= (numLines%numcolors)/4; /* skip this many per side */
  if(numcolors>256)
  {
    colstep = (float)numcolors/useCX;
  }
  for(x=0;x<useCX;x++)
  {
    XDrawLine(display,window[winno],color_gcs[col],
	      useCX-x,useCY,midX,midY);
    if(numberToSkip)
      gaaak=x%(useCX/numberToSkip);
    else
      gaaak=1;
    if(gaaak)
    {
      col+=colstep;
      if(col>numcolors-1)
	col=1;	  
    }
  }
  for(y=0;y<useCY;y++)
  {
    XDrawLine(display,window[winno],color_gcs[col],
	      0,useCY-y,midX,midY);
    if(numberToSkip)
      gaaak=y%(useCY/numberToSkip);
    else
      gaaak=1;
    if(gaaak)
    {
      col+=colstep;
      if(col>numcolors-1)
	col=1;	  
    }
  }
  for(x=0;x<useCX;x++)
  {
    XDrawLine(display,window[winno],color_gcs[col],
	      x,0,midX,midY);
    if(numberToSkip)
      gaaak=x%(useCX/numberToSkip);
    else
      gaaak=1;
      
    if(gaaak)
    {
      col+=colstep;
      if(col>numcolors-1)
	col=1;	  
    }
  }
  for(y=0;y<useCY;y++)
  {
    XDrawLine(display,window[winno],color_gcs[col],
	      useCX,y,midX,midY);
    if(numberToSkip)
      gaaak=y%(useCY/numberToSkip);
    else
      gaaak=1;
    if(gaaak)
    {
      col+=colstep;
      if(col>numcolors-1)
	col=1;	  
    }
  }

  if(options.mirrors)
  {
    for(x=0;x<=(options.mirrors+1);x++)
      for(y=0;y<=(options.mirrors+1);y++)
      {
         XCopyArea(display,window[winno],window[winno],color_gcs[1],
		   0,0,useCX,useCY,useCX*x,useCY*y);
      }  
  }
}

void
exit_test()
{
  return;
}

void
draw_test(int winno)
{
  int x,y;
  int fx,fy;
  float ffx, ffy;
  
  if(options.palette==3)
  {
    fx=fy=(int)sqrt(numcolors);
    ffx =(float)(CX[winno]/sqrt(numcolors));
    ffy =(float)(CY[winno]/sqrt(numcolors));
  }
  else
  {
    fx=(numcolors/6);
    fy=6;
    while(fy<<1<fx>>1) 
    {
      fx>>=1;
      fy<<=1;
    }
    ffx =(CX[winno]/(float)fx);
    ffy =(CY[winno]/(float)fy);
  }

  for (x=0;x<fx;x++)
  {
    for (y=0;y<fy;y++)
    {
      XFillRectangle(display,window[winno],color_gcs[y*fx+x],
		     (int)(x*ffx),(int)(y*ffy),(unsigned int)(ffx+1),
		     (unsigned int)(ffy+1));
    }
    XFlush(display);
  }
}

void
exit_sphere()
{
  return;
}

void
draw_sphere(int winno)
{
  int x2=rndm(CX[winno]);
  int y2=rndm(CY[winno]);
  int x1=rndm(CX[winno]);
  int y1=rndm(CY[winno]);

  HC[winno]=rndm(numcolors);

  while((abs(x1-x2)>1)&&(abs(y1-y2)>1))
  {
    XFillPolygon(display,window[winno],color_gcs[HC[winno]],
		 make_circle(x1, y1, x2, y2),
		 33,Nonconvex,CoordModeOrigin);
    if(x1-x2<0)
    { x1++; x2--; }
    else if (x1-x2>0)
    { x1--; x2++; }
    if(y1-y2<0)
    { y1++; y2--; }
    else if (y1-y2>0)
    { y1--; y2++; }
    HC[winno]=(HC[winno]+1)%numcolors;
    if(HC[winno]==0) HC[winno]=1;
  }
}


void
exit_off()
{
  return;
}

void
draw_off(int winno)
{
  int x,i;
  unsigned int fifthCX,seventhCX,quarterCY;
  XGCValues values;
  /* White, Yellow,   Cyan,     Green,     Magenta,    Red, Blue */
  /* alloc, numcol/6, numcol/2, numcol/3,  5*numcol/6, 1,   2*numcol/3  */
  int foob[8];
  foob[0]= numcolors/12;
  foob[1]= numcolors/6;
  foob[2]= numcolors/2;
  foob[3]= numcolors/3;
  foob[4]= 5*numcolors/6;
  foob[5]= 1;
  foob[6]= 2*numcolors/3;
  
/* make an Orange into a White*/
  values.foreground=StrColor("white",WhitePixel(display,screen));
  XChangeGC(display,color_gcs[numcolors/12],GCForeground,&values);
  
/* and a blue-purple into a Navy */
  values.foreground=StrColor("navy blue",BlackPixel(display,screen));
  XChangeGC(display,color_gcs[9*numcolors/12],GCForeground,&values);
/* and another blue becomes a medium blue */
  values.foreground=StrColor("medium blue",BlackPixel(display,screen));
  XChangeGC(display,color_gcs[1+(9*numcolors/12)],GCForeground,&values);

  fifthCX=(unsigned int)((float)CX[winno]/5);
  quarterCY=(unsigned int)((float)(CY[winno]/4));
  seventhCX=(unsigned int)((float)(CX[winno]/7));
  
  for(i=0,x=0;i<7;i++,x++)
  {
    XFillRectangle(display,window[winno],color_gcs[foob[i]],
		   x*seventhCX, 0, seventhCX,
                   (unsigned int)(3*quarterCY));
  }

  
  /* and the squares across the bottom */
  XFillRectangle(display,window[winno],color_gcs[9*numcolors/12],
		 0,(3*quarterCY), fifthCX, quarterCY);
  
  XFillRectangle(display,window[winno],color_gcs[numcolors/12],
		 fifthCX,(3*quarterCY), fifthCX, quarterCY);
  
  XFillRectangle(display,window[winno],color_gcs[1+(9*numcolors/12)],
		 2*fifthCX,3*quarterCY,fifthCX,quarterCY);
}

void
draw_sunflower(int winno)
{
  int x,y;
  int xcenter = CX[winno]>>1;
  int ycenter = CY[winno]>>1;
  for(x=0;x<CX[winno];x++)
   for(y=0;y<CY[winno];y++)
   {
     unsigned int d = math_dist(x,y,xcenter,ycenter);
/*     int angle = (int)(RecttoPolar(dx, dy)*options.number); */
     int colr =(int)(d* RecttoPolar(x-xcenter,y-ycenter));
     while(colr<0) colr+=numcolors;
     while(colr>=numcolors) colr-=numcolors;
     fprintf(stderr,"colr = %d x = %d y = %d\n",colr,x,y);
     XDrawPoint(display,window[winno],color_gcs[colr],x,y);
   }
  return;
}

void
exit_sunflower(int winno)
{
  return;
}

void
draw_flush(int winno)
{
   int i,x,y;
/* occasionally drop new stuff in */

   if(!rndm(4000))
   {
     int color = rndm(numcolors);
     int xsize = rndm(CX[winno]>>3);
     int ysize = rndm(CY[winno]>>3);
     XFillRectangle(display,window[winno],color_gcs[color],
		    rndm(CX[winno]),rndm(CY[winno]),
		    xsize,ysize);
   }

/* start from the center, and XCopyArea stuff toward the middle, with a
   counter-clockwise spin (maybe clockwise if we find a domain name
   from the southern hemisphere :)*/
   for(i=0;i<20;i++)
   {
     int dx,dy,dist;
     double angle;
     XPoint newpt;
     x=rndm(CX[winno]); y=rndm(CY[winno]);
      dx = x - (CX[winno]>>1);
      dy = y - (CY[winno]>>1);
      dist = math_dist(0,0,dx, dy);
      angle = (RecttoPolar(dx, dy));
      dist -= 5;
      angle += M_PI/64;
      newpt = PolartoRect(dist,angle);
      XCopyArea(display,window[winno],window[winno],color_gcs[0],
		x,y,3,3,newpt.x+(CX[winno]>>1),newpt.y+(CY[winno]>>1));
   }
   return;
}

void
init_flush(int winno)
{
  int i;
  for(i=0;i<60;i++)
  {
     int color = rndm(numcolors);
     int xsize = rndm(CX[winno]>>1);
     int ysize = rndm(CY[winno]>>1);
     XFillRectangle(display,window[winno],color_gcs[color],
		    rndm(CX[winno]),rndm(CY[winno]),
		    xsize,ysize);
  }
  return;
}

void
exit_flush(int winno)
{
  return;
}

void
draw_blur(int winno)
{
   int i,x,y;
/* occasionally drop new stuff in */

   if(!rndm(4000))
   {
     int color = rndm(numcolors);
     int xsize = rndm(CX[winno]>>3);
     int ysize = rndm(CY[winno]>>3);
     XFillRectangle(display,window[winno],color_gcs[color],
		    rndm(CX[winno]),rndm(CY[winno]),
		    xsize,ysize);
   }

   for(i=0;i<20;i++)
   {
     x=rndm(CX[winno]); y=rndm(CY[winno]);
     XCopyArea(display,window[winno],window[winno],color_gcs[0],
	       x,y,2,2,x+(rndm(5))-2,y+(rndm(5))-2);
   }
   return;
}

void
init_blur(int winno)
{
  int i;
  for(i=0;i<60;i++)
  {
    int color = rndm(numcolors);
    int xsize = rndm(CX[winno]>>1);
    int ysize = rndm(CY[winno]>>1);
    XFillRectangle(display,window[winno],color_gcs[color],
		   rndm(CX[winno]),rndm(CY[winno]),
		   xsize,ysize);
  }
  return;
}

void
exit_blur(int winno)
{
  return;
}

#ifdef IMAGE
void
init_munch(int winno)
{
  im = XGetImage(display,window[winno],0,0,CX[winno],CY[winno],~0L,ZPixmap);
  if(im==NULL)
  {
    fprintf(stderr, "Couldn't create Image. Exiting\n");
    exit(1);
  }
}

void
exit_munch()
{
  XDestroyImage(im);
}


void draw_munch(int winno,int spoo)
{
  int x=0;
  int foo;
  
  for(x=0;x<CX[winno];x++)
  {
    foo=XGetPixel(im,x,(x^spoo)%CY[winno])+1;
    if(foo==0||foo>numcolors) foo=1;
    XPutPixel(im,x,(x^spoo++)%CY[winno],foo);
  }
  XPutImage(display,window[winno],color_gcs[0],im,0,0,0,0,CX[winno],CY[winno]);
}
#else
void
init_munch(int winno)
{
  
}

void 
exit_munch()
{
  
}

void
draw_munch(int winno,int spoo)
{
  
}

#endif
