/*
 *  Color Routines.. randomizing, rotating, and init'ing colormap
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "trippy.h"

#ifndef sgn
#define sgn(x) ( ((x)<0)?(-1):(1))
#endif

extern Visual *vis;
extern int visclass;
extern unsigned int depth;
XColor *color_info; /* [NCOLORS]; */
short m_color=1;
static int allocPalette1(int);
static int allocPalette2(int);
static int allocPalette3(int);
static int allocPalette5(int);
static int allocPalette24(int);
/* static int allocPalette7(int); */
static int allocPaletteDefault(int);
static void allocSharedColors();
void fade_to(unsigned int,unsigned int,unsigned int);

unsigned long
StrColor(char *string,unsigned long def)
{  
  /* Lookup a color by its name */
  XColor screen_def_return, exact_def_return;
  if (string==(char*)0 || XAllocNamedColor(display, colmap, string,
					   &exact_def_return,
					   &screen_def_return)==False)
    return def;
  else 
    return screen_def_return.pixel;
}
     
int
randomize_color()
{
/* pick a random color from the list, set it to a random value */
  XColor color;
  int i=rndm((long)numcolors-1);
  if (share_colors) return 0;
  
  if(i==0)
    i=1;  /* overwriting the Background color is a Bad Thing */
  
  color.pixel = color_info[i].pixel;
  colors[i][0]=color.red = rndm(65535L);
  colors[i][1]=color.green = rndm(65535L);
  colors[i][2]=color.blue = rndm(65535L);
  color.flags = DoRed|DoGreen|DoBlue;
  XStoreColor(display, colmap, &color);
  color_info[i].red = color.red;
  color_info[i].green = color.green;
  color_info[i].blue = color.blue;

  return i;
}

void
randomize_colors()
{
  int i;
  /* randomize the whole Colormap */

  {
    XColor *color;
    color=(XColor *)malloc(numcolors*sizeof(XColor));
    for (i=1; i<numcolors; i++) 
    {
      color[i].pixel = color_info[i].pixel; 
      
      if(options.mono)
	{
	  colors[i][0]=color[i].red = 
	    colors[i][1]=color[i].green =
	    colors[i][2]=color[i].blue = rndm(65535L);
	}
      else
	{
	  colors[i][0]=color[i].red = rndm(65535L);
	  colors[i][1]=color[i].green = rndm(65535L);
	  colors[i][2]=color[i].blue = rndm(65535L);
	}
      
      color[i].flags = DoRed|DoGreen|DoBlue;
      if(! share_colors)
        XStoreColor(display, colmap, &color[i]); 
      color_info[i].red = color[i].red;
      color_info[i].green = color[i].green;
      color_info[i].blue = color[i].blue;
    }
    free (color);
    /*  XStoreColors(display,colmap, &color[1], numcolors-1); */
  }
}

void
store_colors(int nc,int* pal)
{
  int i;

  if(nc>numcolors)  nc = numcolors;  /* don't try to do more than we have */
  
  
  {
    XColor *color;
    color=(XColor *)malloc(nc*sizeof(XColor));
    for (i=1; i<nc; i++) 
    {
      color[i].pixel = color_info[i].pixel; 
      
      if(options.mono)
	{
	  colors[i][0]=color[i].red = 
	    colors[i][1]=color[i].green =
	    colors[i][2]=color[i].blue = pal[(i*3)+1];
	}
      else
	{
	  colors[i][0]=color[i].red = pal[(i*3)];
	  colors[i][1]=color[i].green = pal[(i*3)+1];
	  colors[i][2]=color[i].blue =  pal[(i*3)+2];
	}
      
      color[i].flags = DoRed|DoGreen|DoBlue;
      if(! share_colors)
        XStoreColor(display, colmap, &color[i]); 
      color_info[i].red = color[i].red;
      color_info[i].green = color[i].green;
      color_info[i].blue = color[i].blue;
    }
    free (color);
    /*  XStoreColors(display,colmap, &color[1], numcolors-1); */
  }
}

void
make_white(int entry)
{
   if (share_colors)
   {
     XGCValues gcval;
     gcval.foreground=WhitePixel(display,screen);
     XChangeGC(display,color_gcs[entry],GCForeground,&gcval);
   }
   else
   {
     XColor col;
     col.flags=DoRed|DoGreen|DoBlue;
     col.pixel= color_info[entry].pixel;
     col.red=col.blue=col.green=65535;   /* WHITE */
     XStoreColor(display,colmap,&col);
   }
}


void
rotate_colors()
{ 
/*  Rotate the colormaps
 *  Palette 0 is the standard RYGCyBM
 *  Palette 1 is 3 band of Red, Green and Blue
 *  Palette 2 starts out Greyscale
 *  Palette 3 starts out... well, it's complex
 *  Palette 4 starts like palette 0, then interjects random new
 *   colors, and interpolates
 *  Palette 5 drops 3 random colors , and interpolates between 'em
 *  in rotating palettes 1 and 2, the 3 primary colors (RGB) are independent
 *  of each other...
 */

  XColor temp_color;
  register int i;
  static int plusme1=0,plusme2=0,plusme3=0;
  static int add1=1,add2=1,add3=1;
  int a,b,c;
  
  if (share_colors) return;
  memcpy(&temp_color,&color_info[1],sizeof(XColor));
  if(options.palette==4)
    {
      static int smoothing=0;
      static int ind;
      static int steps,nSteps;
      static float forR, forG, forB;
      static float bacR, bacG, bacB;
      
      if(!smoothing)
	{
	  int iF,iB;
	  
	  ind=randomize_color();  /* change one */
	  steps = rndm(50l)+1;
	  iF = (ind+steps)%numcolors;
	  iB = (ind-steps)%numcolors;
	  if(iB<=0)
	    iB = numcolors-1+iB;
/*
  fprintf(stdout,"\t ind= %d iF = %d iB = %d steps=%d\n",ind,iF,iB,steps);
  fprintf(stdout,"\tcolors[ind]= (%u,%u,%u)\n",
  colors[ind][0],colors[ind][1],colors[ind][2]);
  fprintf(stdout,"\tcolors[iF]= (%u,%u,%u) colors[iB]= (%u,%u,%u)\n",
  colors[iF][0],colors[iF][1],colors[iF][2],
  colors[iB][0],colors[iB][1],colors[iB][2]);
  */	  
/* wow, this looks gross! but the version of GCC I was working with couldn't
 *  deal with everything on the same line, so....
 */
	  forR = (float)(colors[iF][0]-colors[ind][0]);
	  forR /= (float)steps;
	  forG = (float)(colors[iF][1]-colors[ind][1]);
	  forG /= (float)steps;
	  forB = (float)(colors[iF][2]-colors[ind][2]);
	  forB /= (float)steps;
	  bacR = (float)(colors[iB][0]-colors[ind][0]);
	  bacR /= (float)steps;
	  bacG = (float)(colors[iB][1]-colors[ind][1]);
	  bacG /= (float)steps;
	  bacB = (float)(colors[iB][2]-colors[ind][2]);
	  bacB /= (float)steps;
/*
   fprintf(stdout,"\tfor = (%6.2f,%6.2f,%6.2f)\n bac = (%6.2f,%6.2f,%6.2f)\n",
   forR,forG,forB,bacR,bacG,bacB);
 */     
	  nSteps=1;
	  smoothing=1;
	}
      else
	{
	  if(nSteps==steps)
	    smoothing=0;
	  else
	    {
	      int iF = (ind+nSteps)%numcolors;
	      int iB = (ind-nSteps)%numcolors;
	      if(iF==0) iF=1;
	      if(iB<=0)
		iB = numcolors-1+iB;
	      
	      colors[iF][0]= colors[ind][0] +(unsigned long)(forR*nSteps);
	      colors[iF][1]= colors[ind][1] +(unsigned long)(forG*nSteps);
	      colors[iF][2]= colors[ind][2] +(unsigned long)(forB*nSteps);
	      colors[iB][0]= colors[ind][0] +(unsigned long)(bacR*nSteps);
	      colors[iB][1]= colors[ind][1] +(unsigned long)(bacG*nSteps);
	      colors[iB][2]= colors[ind][2] +(unsigned long)(bacB*nSteps);

	      color_info[iF].red   = colors[iF][0];
	      color_info[iF].green = colors[iF][1];
	      color_info[iF].blue  = colors[iF][2];
	      color_info[iF].flags = DoRed|DoGreen|DoBlue;
/*
	fprintf(stdout, "Colors[%d] = (%u,%u,%u)... \n",
		iF,colors[iF][0],colors[iF][1],colors[iF][2]);
*/			
	      XStoreColor(display,colmap,&color_info[iF]);

	      color_info[iB].red   = colors[iB][0];
	      color_info[iB].green = colors[iB][1];
	      color_info[iB].blue  = colors[iB][2];
	      color_info[iB].flags = DoRed|DoGreen|DoBlue;
/*
  fprintf(stdout, "Colors[%d] = (%u,%u,%u)...\n", iB,
  colors[iB][0],colors[iB][1],colors[iB][2]);
*/ 
	      XStoreColor(display,colmap,&color_info[iB]);
	      
	      nSteps++;
	    }
	}
      return;
    }
  else /* every palette except #4 */
    {
      for (i=1; i<numcolors; i++)
	{
	  if(options.palette!=0 &&options.palette!=5)
	    {
	      a= (i+plusme1)%numcolors;
	      b= (i+plusme2)%numcolors;
	      c= (i+plusme3)%numcolors;
	      if(a<0) a=numcolors+a;
	      if(b<0) b=numcolors+b;
	      if(c<0) c=numcolors+c;
	      color_info[i].red=colors[a][0];
	      color_info[i].green=colors[b][1];
	      color_info[i].blue=colors[c][2];
	    }
	  else
	    color_info[i].pixel = color_info[i+1].pixel;
	  color_info[i].flags = DoRed|DoGreen|DoBlue;
	  /*      XStoreColor(display,colmap,&color_info[i]); */
	}
      if(options.palette==0 ||options.palette==5)
	{
	  memcpy(&color_info[numcolors-1],&temp_color,sizeof(XColor));
	  /*      color_info[numcolors-1].pixel = temp_color.pixel;
	   *     color_info[numcolors-1].flags = DoRed|DoGreen|DoBlue; 
	   */
	}
      else
	{
	  if(!rndm(5000l))
	    add1*= -1;
	  plusme1+=add1;
	  if(!rndm(5000l))
	    add2*= -1;
	  plusme2+=add2;
	  if(!rndm(5000l))
	    add3*= -1;
	  plusme3+=add3;
	}
      XStoreColors(display,colmap,&color_info[1],numcolors-1); 
    }
}

void
fade_to_black()
{
  fade_to(0,0,0);
}

void
fade_to_white()
{
  fade_to(65535,65535,65535);
}

void
fade_to(unsigned int r, unsigned int g, unsigned  int b)
{
  int i;
  int done=0;
  int steps = rndm(150l)+300;


  while(done<steps)
    {
      for(i=0;i<numcolors;i++)
	{     
	  if(color_info[i].red!=r)
	    {
	      color_info[i].red=color_info[i].red+
		((color_info[i].red-r)/steps);
	    }
	  if(color_info[i].green!=g)
	    {
	      color_info[i].green=color_info[i].green+
		((color_info[i].green-g)/steps);
	    }
	  if(color_info[i].blue!=b)
	    {
	      color_info[i].blue=color_info[i].blue+
		((color_info[i].blue-b)/steps);
	    }
	  /*
	    fprintf(stderr,"Color_info[%d] = (%u,%u,%u)\n",i,color_info[i].red,
	    color_info[i].green, color_info[i].blue);;
	    */
	}
      XStoreColors(display,colmap,&color_info[1],numcolors-1); 
      done++;
    }
 
  return;
}

void
get_them_colors()
{
  /* Allocate the Colormap and Initialize them */
  unsigned long *pixels; /*[NCOLORS];  */
  unsigned long *plane_masks=0;
  int i;
  int mask_for;
  XGCValues values;  
  pixels=(unsigned long *)malloc(sizeof(unsigned long)*options.tryfor);
/*
  if(options.perfect)
  mask_for=1;
  else
  */
  mask_for=0;

  if (visclass==0)
    {
      color_info=(XColor *)malloc(sizeof(XColor)*2);
      color_gcs=(GC *)malloc(2*sizeof(GC));
      numcolors=2;
      color_info[0].pixel = WhitePixel(display,screen);
      color_info[1].pixel = BlackPixel(display,screen);
      for(i=0;i<numcolors;i++)
	{
	  values.foreground = color_info[i].pixel;
	  values.background = options.bgcolor;
	  color_gcs[i]=XCreateGC(display,window[0],
				 GCForeground|GCBackground,&values);
	}
    }
  else if (options.dynamic_colors) 
    {
      for (numcolors=options.tryfor; numcolors>=2; numcolors-=6) 
	{
	  if (XAllocColorCells(display,colmap, True , plane_masks,
			       mask_for, pixels,
			       (unsigned int)numcolors) != 0) 
	    {
	      color_info=(XColor *)malloc(sizeof(XColor)*(numcolors+1));
	      colors=(long**)malloc(numcolors*sizeof(long*));
	      for (i=0;i<numcolors;i++)
		{
		  color_info[i].pixel=pixels[i];
		  colors[i]=(long*)malloc(3*sizeof(long));
		}
	      
	      randomize_colors();
	      break;
	    }
	}
      
      fprintf(stderr,"Alloc'ing %d colors \n",numcolors);
      if (numcolors < 2)
	{
	  fprintf(stderr,"Unable to alloc my own colors.\n");
	  allocSharedColors();
	}
      
      if ((HC =(unsigned int *)calloc(options.windows,sizeof(unsigned int)))
	  == NULL) return ;
      color_gcs = (GC *)malloc(numcolors*sizeof(GC));
      
      color_info[0].pixel=options.bgcolor;
      for(i=0;i<numcolors;i++)
	{
	  int win=0;
	  values.foreground = color_info[i].pixel;
	  values.background = options.bgcolor;
	  for(win=0;win<options.windows;win++)
	    color_gcs[i] = XCreateGC(display,window[win],
				     GCForeground|GCBackground,&values);
	}
    }
  else if(!options.mono)
    {
      XColor screen_in_out;
      int nc=options.tryfor;
      int done=0;
      screen_in_out.flags=DoRed|DoGreen|DoBlue;
    
      while(!done)
	{
	  if (nc<=0)
	    {
	      fprintf(stderr,"Doh! Cannot allocate any color cells\n");
	      /* exit(1); */
	      allocSharedColors();
	      return;
	    }
	  if (XAllocColorCells(display, colmap, True, plane_masks,
			       mask_for, pixels, nc) != 0) 
	    {
	      color_info=(XColor *)malloc(sizeof(XColor)*(nc+1));
	      colors=(long**)malloc((nc+1)*sizeof(long*));
	      for(i=0;i<=nc;i++)
		{
		  colors[i]=(long*)calloc(3,sizeof(long));
		}
	
	      switch (options.palette)
		{
		case 3:
		  {
		    nc=allocPalette3(nc);
		    break;
		  }
		case 24:
		  {
		    nc=allocPalette24(nc);
		    break;
		  }
		case 5:
		  {
		    allocPalette5(nc);
		    break;
		  }
		case 2:
		case 4:
		  {
		    allocPalette2(nc);
		    break;
		  }
		case 1:
		  {
		    allocPalette1(nc);
		    break;
		  }
		case 0:
		default:
		  {
		    allocPaletteDefault(nc);
		    break;
		  }
		}
	      done=1;
	    }
	  else
	    nc-=6;
	}
      
      fprintf(stderr,"Alloc'ing %d colors \n",nc);
      
      if ((HC =(unsigned int *)calloc(options.windows,sizeof(unsigned int)))
	  == NULL) return ;
      color_gcs = (GC *)malloc(nc*sizeof(GC));
      
      for(numcolors=0;numcolors<nc;numcolors++)
	{
	  /*   int win; */
	  screen_in_out.red   = colors[numcolors][0];
	  screen_in_out.green = colors[numcolors][1];
	  screen_in_out.blue  = colors[numcolors][2];
	  /*
	    fprintf (stderr,"Colors(%ld,%ld,%ld)\n",colors[numcolors][0],
	    colors[numcolors][1], colors[numcolors][2]);
	    */
	  screen_in_out.pixel = pixels[numcolors];
     
	  
	  if (XStoreColor(display, colmap, &screen_in_out)==False) 
	    {
	      /* oh well... guess this doesn't mean anything now */
	      /*	  fatalerror("Cannot allocate colors",""); */
	    }
	  
	  color_info[numcolors].pixel = screen_in_out.pixel;
	  color_info[numcolors].red = screen_in_out.red;
	  color_info[numcolors].green = screen_in_out.green;
	  color_info[numcolors].blue = screen_in_out.blue;
	  color_info[0].pixel=options.bgcolor;
	  values.foreground = color_info[numcolors].pixel;
	  values.background = options.bgcolor;
	  /*    for(win=0;win<options.windows;win++) */
	  color_gcs[numcolors] = XCreateGC(display,window[0],
					   GCForeground|GCBackground,
					   &values);
	}
    }
  else                                           /*greyscale */
    { 
      XColor screen_in_out;
      int nc=options.tryfor;
      int done=0;
      screen_in_out.flags=DoRed|DoGreen|DoBlue;
      
      while(!done)
	{
	  if (XAllocColorCells(display,colmap, True, plane_masks, mask_for,
			       pixels, nc) != 0) 
	    {
	      color_info=(XColor *)malloc(sizeof(XColor)*nc);
	      for (numcolors=0; numcolors<nc; numcolors++) 
		{
		  int win;
		  colors[numcolors][0]=
		    colors[numcolors][1]=
		    colors[numcolors][2]=
		    (unsigned long)(numcolors*(float)(65535/nc));
		  
		  screen_in_out.flags = DoRed | DoGreen | DoBlue;
		  screen_in_out.red = colors[numcolors][0];
		  screen_in_out.green = colors[numcolors][1];
		  screen_in_out.blue = colors[numcolors][2];
		  screen_in_out.pixel = pixels[numcolors];	  
		  if (XStoreColor(display, colmap,&screen_in_out)==False) 
		    {
		      /*
			if (numcolors < 2) 
			fatalerror("Cannot allocate colors","");
			break;
		      */
		    }
		  color_info[numcolors].pixel = screen_in_out.pixel;
		  color_info[numcolors].red = screen_in_out.red;
		  color_info[numcolors].green = screen_in_out.green;
		  color_info[numcolors].blue = screen_in_out.blue;
		  values.foreground = color_info[numcolors].pixel;
		  values.background = options.bgcolor;
		  for(win=0;win<options.windows;win++)
		    color_gcs[numcolors] = XCreateGC(display,window[win],
						     GCForeground|GCBackground,
						     &values);
		}
	      done=1;
	    }
	  else
	    nc-=6;
	}
      fprintf(stderr,"Alloc'ing %d colors \n",nc);
    }
  free (pixels);
}

int
allocPalette2(int nc)
{
  for(numcolors=0;numcolors<=nc>>1;numcolors++)
    {
      colors[numcolors][0]=
	colors[nc-numcolors][0]=
        colors[numcolors][1]=
	colors[nc-numcolors][1]=
	colors[numcolors][2]=
	colors[nc-numcolors][2]=
	(unsigned long)(numcolors*2*(float)(65535/nc));
    }
  numcolors=nc;
  return numcolors;
}

int
allocPalette1(int nc)
{
  float fact = (float)65535/(nc/6); 
  for(numcolors=0;numcolors<=(nc/6);numcolors++) 
    { 
      colors[numcolors][0]= 
	colors[numcolors+(long)(nc*2/6)][1]= 
	colors[numcolors+(long)(nc*4/6)][2]= 
	(unsigned long)(fact*numcolors); 
      colors[numcolors+(long)(nc/6)][0]= 
	colors[numcolors+((long)nc*3/6)][1]= 
	colors[numcolors+(long)(nc*5/6)][2]= 
	(unsigned long)(65535-(fact*numcolors)); 
      /* and then we zero out anything else */
      colors[numcolors][2]= 
	colors[numcolors+(long)(nc/6)][2]= 
	colors[numcolors+((long)nc*2/6)][0]= 
	colors[numcolors+(long)(nc*3/6)][0]=
	colors[numcolors+((long)nc*4/6)][1]= 
	colors[numcolors+((long)nc*5/6)][1]=
	(unsigned long)0; 
    }
  numcolors=nc;
  return nc;
}

int
allocPaletteDefault(int nc)
{
  float fact = (float)65535/(nc/6);
  for(numcolors=0;numcolors<=(nc/6);numcolors++) 
    { 
      colors[numcolors][1]= 
	colors[numcolors+(long)(nc*2/6)][2]= 
        colors[numcolors+(long)(nc*4/6)][0]=
	(unsigned long)(fact*numcolors); 
      colors[numcolors+(nc/6)][0]= 
	colors[numcolors+(long)(nc*3/6)][1]= 
        colors[numcolors+((long)nc*5/6)][2]=
	(unsigned long)(65535-(fact*numcolors)); 
      colors[numcolors][0]= 
	colors[numcolors+(long)(nc/6)][1]= 
        colors[numcolors+(long)(nc*2/6)][1]= 
	colors[numcolors+(long)(nc*3/6)][2]= 
	colors[numcolors+(long)(nc*4/6)][2]= 
	colors[numcolors+(long)(nc*5/6)][0]=
	(unsigned long)65535; 
    /* and then we zero out anything else */
      colors[numcolors][2]= 
	colors[numcolors+(long)(nc/6)][2]= 
        colors[numcolors+(long)(nc*2/6)][0]= 
	colors[numcolors+(long)(nc*3/6)][0]=
	colors[numcolors+(long)(nc*4/6)][1]= 
	colors[numcolors+(long)(nc*5/6)][1]=  (unsigned long)0; 
    }
  colors[0][0]=0;  colors[0][1]=0;  colors[0][2]=0;
  
  numcolors=nc;
  return nc;
}

int
allocPalette3(int nc)
{
  int stepX,stepY;
  int sqrColors,done,x,y;
  
  sqrColors=(int)sqrt(nc); /* yes, we lose some decimal points here.
			      Good riddance! */
  done=0;
  x=sqrColors;
  y=(sqrColors-1)*sqrColors;
  
  do
    {
      colors[x][0]=rndm(65535);
      colors[x][1]=rndm(65535);
      colors[x][2]=rndm(65535);
      
      colors[y][0]=rndm(65535);
      colors[y][1]=rndm(65535);
      colors[y][2]=rndm(65535);
      
      /* make sure at least one of the colors is fairly strong */
      if(( (colors[x][0]<32767) &&
	   (colors[x][1]<32767) &&
	   (colors[x][2]<32767) ) ||
	 ( (colors[y][0]<32767) &&
	   (colors[y][1]<32767) &&
	   (colors[y][2]<32767) ) ||
	 ( colors[x][0]+colors[y][0]>65535) ||
	 ( colors[x][1]+colors[y][1]>65535) ||
	 (colors[x][2]+colors[y][2]>65535) )
	done=0;
      else done=1;
    } while (!done);
  
  colors[0][0]=
    colors[0][1]=
    colors[0][2]=0;
  
  for(stepX=1;stepX<x;stepX++)
    {
      colors[stepX][0]=(unsigned long)(colors[x][0]*((float)stepX/x));
      colors[stepX][1]=(unsigned long)(colors[x][1]*((float)stepX/x));
      colors[stepX][2]=(unsigned long)(colors[x][2]*((float)stepX/x));
    }
			
  for(stepY=1;stepY<x;stepY++)
    {
      colors[stepY*x][0]=(unsigned long)(colors[y][0]*((float)stepY/x));
      colors[stepY*x][1]=(unsigned long)(colors[y][1]*((float)stepY/x));
      colors[stepY*x][2]=(unsigned long)(colors[y][2]*((float)stepY/x));
    }
  
  for(stepX=1;stepX<x;stepX++)
    for(stepY=1;stepY<x;stepY++)
      {
	colors[stepX+(stepY*x)][0]= 
	  colors[stepX][0] + colors[stepY*x][0];
	colors[stepX+(stepY*x)][1]= 
	  colors[stepX][1] + colors[stepY*x][1];
	colors[stepX+(stepY*x)][2]= 
	  colors[stepX][2] + colors[stepY*x][2];
      }
  
  return (sqrColors*sqrColors);
}

int
allocPalette24(int nc)
{
  int i;
  int maxval=0;
  FILE *fp;

  if((fp=fopen(options.palette_filename,"r"))==NULL)
    {
      fprintf(stderr," Oop.. can't open Palette file %s\n",
	      options.palette_filename);
      exit(1); 
    }
  else
    {
      char buffer [255]; 
      /* long reported; */
      i=1;
      /* fscanf(fp,"%ld\n",&reported); */
      while(fgets(buffer,255,fp)!=NULL)
	{
	  sscanf(buffer,"%ld %ld %ld",&colors[i][0],&colors[i][1],
		 &colors[i][2]);
	  if(colors[i][0]>maxval) maxval=colors[i][0];
	  if(colors[i][1]>maxval) maxval=colors[i][1];
	  if(colors[i][2]>maxval) maxval=colors[i][2];
	  i++;
	  if(i>=nc)
	    break;
	}
      fclose(fp);
    }
  nc=i;
  if(maxval<256)
    {
      for(i=1; i<nc;i++)
	{
	  colors[i][0]*=256;
	  colors[i][1]*=256;
	  colors[i][2]*=256;
	}
    }
  return nc;
}

int
allocPalette5(int nc)
{
  int dR1,dR2,dR3;
  int dG1,dG2,dG3;
  int dB1,dB2,dB3;
  const int x1 = nc/3;
  const int x2 = 2*nc/3;

  colors[1][0] = rndm(65535l);
  colors[1][1] = rndm(65535l);
  colors[1][2] = rndm(65535l);
  colors[x1][0] = rndm(65535l);
  colors[x1][1] = rndm(65535l);
  colors[x1][2] = rndm(65535l);
  colors[x2][0] = rndm(65535l);
  colors[x2][1] = rndm(65535l);
  colors[x2][2] = rndm(65535l);

  dR1 = (colors[x1][0] - colors[1][0]);
  dR1 /= x1;			  
  dG1 = (colors[x1][1] - colors[1][1]);
  dG1 /= x1;
  dB1 = (colors[x1][2] - colors[1][2]);
  dB1 /= x1;

  dR2 = (colors[x2][0] - colors[x1][0]);
  dR2 /= x1;
  dG2 = (colors[x2][1] - colors[x1][1]);
  dG2 /= x1;			  
  dB2 = (colors[x2][2] - colors[x1][2]);
  dB2 /= x1;			  
  dR3 = (colors[1][0] - colors[x2][0]);
  dR3 /= x1;			  
  dG3 = (colors[1][1] - colors[x2][1]);
  dG3 /= x1;			  
  dB3 = (colors[1][2] - colors[x2][2]);
  dB3 /= x1;

  /*
  fprintf(stderr,"nc= %d, x1=%d x2=%d\n",nc,x1,x2);
  fprintf(stderr,"d1 = (%d,%d,%d)\n",dR1,dB1,dG1);
  fprintf(stderr,"d2 = (%d,%d,%d)\n",dR2,dB2,dG2);
  fprintf(stderr,"d3 = (%d,%d,%d)\n",dR3,dB3,dG3);
  */
  
  for(numcolors=1;numcolors<=x1;numcolors++)
    {
      colors[1+numcolors][0]=(unsigned long)(colors[1][0]+(dR1*numcolors));
      colors[1+numcolors][1]=(unsigned long)(colors[1][1]+(dG1*numcolors));
      colors[1+numcolors][2]=(unsigned long)(colors[1][2]+(dB1*numcolors));
      
      colors[x1+numcolors][0]=(unsigned long)(colors[x1][0]+(dR2*numcolors));
      colors[x1+numcolors][1]=(unsigned long)(colors[x1][1]+(dG2*numcolors));
      colors[x1+numcolors][2]=(unsigned long)(colors[x1][2]+(dB2*numcolors));

      colors[x2+numcolors][0]=(unsigned long)(colors[x2][0]+(dR3*numcolors));
      colors[x2+numcolors][1]=(unsigned long)(colors[x2][1]+(dG3*numcolors));
      colors[x2+numcolors][2]=(unsigned long)(colors[x2][2]+(dB3*numcolors));
    }
  numcolors=nc;
  return nc;
}

void
allocPhatColors()
{
  int r_mask, g_mask, b_mask;
  int r_shift=0, g_shift=0, b_shift=0;
  int r_bits=0, g_bits=0, b_bits=0;
  int redplus=1, greenplus=1, blueplus=1;
  int i, red,blue,green;
  XGCValues values;  

  fprintf(stderr, "Allocating Phat Colors");
  share_colors=1; 

  r_mask = vis->red_mask;
  while( !(r_mask & 1) )
  {
    r_mask >>= 1;
    r_shift++;
  }
  while( r_mask & 1 )
  {
    r_mask >>= 1;
    r_bits++;
  }

  g_mask = vis->green_mask;
  while( !(g_mask & 1) )
  {
    g_mask >>= 1;
    g_shift++;
  }
  while( g_mask & 1 )
  {
    g_mask >>= 1;
    g_bits++;
  }

  b_mask = vis->blue_mask;
  while( !(b_mask &1) )
  {
    b_mask >>= 1;
    b_shift++;
  }
  while( b_mask & 1 )
  {
    b_mask >>= 1;
    b_bits++;
  }

/* hmmm... I should probably limit this to something sane-ish like
 * 4096 colors
 */
  numcolors=1;
  numcolors<<=r_bits;
  numcolors<<=g_bits;
  numcolors<<=b_bits;
  
  fprintf(stderr,"Alloc'ing %d colors",numcolors);
  if(numcolors>4096)
  {
    fprintf(stderr,"\tuh-oh.. going way past my limit.. backing up to 4096 colors\n");
    numcolors=4096;
    blueplus<<=(b_bits-4);
    greenplus<<=(g_bits-4);
    redplus<<=(r_bits-4);
  }
  color_info=(XColor *)malloc(sizeof(XColor)*numcolors);
  colors=(long**)malloc(numcolors*sizeof(long*));
  color_gcs = (GC *)malloc(numcolors*sizeof(GC));
  HC = (unsigned int*)malloc(options.windows*sizeof(unsigned int));
  if (HC == NULL)
  {
    fprintf(stderr,"Aieeee.. memory problem alloc'ing HC\n");
  }
  for(i=0;i<=numcolors;i++)
  {
    colors[i]=(long*)calloc(3,sizeof(long));
  }
  
  i=0;

  if (options.dynamic_colors)
  {
    randomize_colors();
  }
  else if (options.palette==24)
  {
    numcolors=allocPalette24(numcolors);
  }
  else if (options.palette==5) 
  {
    allocPalette5(numcolors);
  }
  else if (options.palette==3)
  {
    numcolors=allocPalette3(numcolors);
  }
  else if (options.palette==2 || options.palette==4)
  {
    allocPalette2(numcolors);
  }
  else if (options.palette==1)
  {
    allocPalette1(numcolors);    
  }
  else
  {
    allocPaletteDefault(numcolors);
  }

  fprintf(stderr,"Numcolors after the alloc = %d\n",numcolors);  
  fprintf(stderr,"plusses=(%d,%d,%d)\n",redplus,greenplus,blueplus);
  fprintf(stderr,"max = (%d,%d,%d)\n",(1<<r_bits),(1<<g_bits),(1<<b_bits));

  for(i=0;i<numcolors;i++)
  {
/*    color_info[i].pixel=i; */
    red=colors[i][0]>>(16-r_bits);
    green=colors[i][1]>>(16-g_bits);
    blue=colors[i][2]>>(16-b_bits);

    color_info[i].pixel = ((red << r_shift) & vis->red_mask) |
			  ((green << g_shift) & vis->green_mask) |
			  ((blue << b_shift) & vis->blue_mask);
    color_info[i].red = red;
    color_info[i].green = green;
    color_info[i].blue = blue;
/*
    fprintf (stderr,"(%d,%d,%d)\n",red,green,blue);
    fprintf (stderr,"Colors(%d,%d,%d)\n",colors[i][0], colors[i][1],
					 colors[i][2]);
*/
  }

  for(i=0;i<numcolors;i++)
  {
    int win;
    values.foreground = color_info[i].pixel;
    values.background = options.bgcolor;
           
    for(win=0;win<options.windows;win++)
      color_gcs[i] = XCreateGC(display,window[win],
			       GCForeground|GCBackground,&values);
  }
  return;
}

void
allocSharedColors()
{
  int i;
  XGCValues values;  

  if ( depth > 8 )
  {
    allocPhatColors(); 
    return;
  }
/*  fprintf(stderr, "Sharing colors with other programs\n"); */
  
  share_colors=1; 
  numcolors = DisplayCells(display,screen);
  color_info=(XColor *)malloc(sizeof(XColor)*numcolors);
  colors=(long**)malloc(numcolors*sizeof(long*));
  color_gcs = (GC *)malloc(numcolors*sizeof(GC));
  HC = (unsigned int*)calloc(options.windows,sizeof(unsigned int));
  for(i=0;i<numcolors;i++)
  {
    color_info[i].pixel=i;
    colors[i]=(long*)malloc(3*sizeof(long));
  }
  XQueryColors(display,colmap,color_info,numcolors); 
/*  if(vis)
  {
    fprintf(stderr,"%lx %lx %lx\n",vis->red_mask,vis->green_mask,vis->blue_mask);
  }
*/
  printf("Sharing %d colors\n",numcolors);
  color_info[0].pixel = BlackPixel(display,screen);
  
  for(i=0;i<numcolors;i++)
  {
    int win;
    values.foreground = color_info[i].pixel;
    values.background = options.bgcolor;
    for(win=0;win<options.windows;win++)
      color_gcs[i] = XCreateGC(display,window[win],
			       GCForeground|GCBackground,&values);
  }
}

#if 0

int
myfunc (int in, int max)
{
  int tmp;

  tmp = (sin (in * M_PI / (max*3/4) ) * max);
  fprintf(stderr, "tmp=%d\n",tmp);

  if (tmp<0)
    return 0;
  else
    return tmp;
}

/* this is Prof's Color setting code.. Good, but it skips Yellow and Cyan */
void
profPalette()
{
  color_info=(XColor *)malloc(sizeof(XColor)*nc);
  for(i=0;i<=nc;i++)                             
  {                                                 
    int num;                                        
    num = myfunc (i, nc);                      
    colors[i][0] = (256 * num / nc) << 8;      
    if (m_color)                                    
    {                                             
      num = i + nc/3;                        
      if (num>nc)                            
	num -= nc;                           
      num = myfunc(num, nc);                 
    }                                             
    colors[i][1] = (256 * num / nc) << 8;      
    if (m_color)                                    
    {                                             
      num = i + 2*nc/3;                      
      if (num>nc)                            
	num -= nc;                           
      num = myfunc(num,nc);                  
    }                                             
    colors[i][2] = (256 * num / nc) << 8;      
  }

#endif
