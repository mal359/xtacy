/*
 *  Rose mode by Noah Vawter (shifty@gweep.net) and
 *		 Jer Johnson (jer@gweep.net)
 */

#include<X11/X.h>
#include<X11/Xlib.h>
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"trippy.h"
#include"lmath.h"

static int inited=0;

#define QQ 0.05
#define RR 0.0
#define THREED 0

#define NUMPTS 128

/*static long **nx1, **ny1, **ox1, **oy1; */
static XPoint **o1, **n1;

#if THREED
/* static long **nx2, **ny2,**ox2, **oy2; */
static XPoint **o2, **n2;
#endif

static long ***phase;  /* phase for 3 harmonics, 3 dimensions */
static long ***phase_add;  /* adjustments */
static int colr;
static int my_col;

void
init_rose()
{
  int winno,x,y;

  o1 =(XPoint**)calloc(options.windows,sizeof(XPoint*));
  n1 =(XPoint**)calloc(options.windows,sizeof(XPoint*));
/*
  nx1=(long**)calloc(options.windows,sizeof(long*));
  ny1=(long**)calloc(options.windows,sizeof(long*));
  ox1=(long**)calloc(options.windows,sizeof(long*));
  oy1=(long**)calloc(options.windows,sizeof(long*));
  */
#if THREED
  o2 =(XPoint**)calloc(options.windows,sizeof(XPoint*));
  n2 =(XPoint**)calloc(options.windows,sizeof(XPoint*));
  /*
  nx2=(long**)calloc(options.windows,sizeof(long*));
  ny2=(long**)calloc(options.windows,sizeof(long*));
  ox2=(long**)calloc(options.windows,sizeof(long*));
  oy2=(long**)calloc(options.windows,sizeof(long*));
  */
#endif
  phase=(long***)calloc(options.windows,sizeof(long**));
  phase_add=(long***)calloc(options.windows,sizeof(long**));
  
  for(winno=0;winno<options.windows;winno++)
  {
    o1[winno] =(XPoint*)calloc(4*NUMPTS,sizeof(XPoint));
    n1[winno] =(XPoint*)calloc(4*NUMPTS,sizeof(XPoint));
    /*
    nx1[winno]=(long*)calloc(4*NUMPTS,sizeof(long));
    ny1[winno]=(long*)calloc(4*NUMPTS,sizeof(long));
    ox1[winno]=(long*)calloc(4*NUMPTS,sizeof(long));
    oy1[winno]=(long*)calloc(4*NUMPTS,sizeof(long));
    */
#if THREED
    o2[winno] =(XPoint**)calloc(options.windows,sizeof(XPoint*));
    n2[winno] =(XPoint**)calloc(options.windows,sizeof(XPoint*));
    /*
    nx2[winno]=(long*)calloc(4*NUMPTS,sizeof(long));
    ny2[winno]=(long*)calloc(4*NUMPTS,sizeof(long));
    ox2[winno]=(long*)calloc(4*NUMPTS,sizeof(long));
    oy2[winno]=(long*)calloc(4*NUMPTS,sizeof(long));
    */
#endif
  
    phase[winno]=(long**)malloc(3*sizeof(long*));
    phase_add[winno]=(long**)malloc(3*sizeof(long*));

    for(y=0;y<3;y++)
    {
      phase[winno][y]=(long*)malloc(options.number*sizeof(long));
      phase_add[winno][y]=(long*)malloc(options.number*sizeof(long));

      for(x=0;x<options.number;x++)
      {
	phase[winno][y][x] =0;
	phase_add[winno][y][x] =  rndm(16);
	fprintf(stderr,"%ld\n",phase_add[winno][y][x]);
      }
    }
  }
  my_col = colr = rndm(numcolors);

  inited=1;
}

void
exit_rose()
{
  int y;
  free(n1);
  free(o1);
  /*
  free(nx1);
  free(ny1);
  free(ox1);
  free(oy1);
  */
#if THREED
  free(n2);
  free(o2);
  /*
  free(nx2);
  free(ny2);
  free(ox2);
  free(oy2);
  */
#endif
  
  for(y=0;y<3;y++)
  {
    free(phase[y]);
    free(phase_add[y]);
  }
  free(phase);
  free(phase_add);
  inited=0; 
}

void
draw_rose(int winno)
{
  int x,y,h,t,num,speed1;
  static int loops=0;
  long px,py,pz;
  XPoint* temp;
  
  int midCX = CX[winno]>>1;
  int midCY = CY[winno]>>1;

  /* calculate 128 points for each side?? */
  for(t=0,num=0;num<NUMPTS;t+=(4096/NUMPTS),num+=1)
  {
    px=0;py=0;pz=0;
    for(h=0;h<(options.number);h++)
    {
      speed1=(h+1)*t;
      px+= mSin(speed1+phase[winno][0][h])/(h+1);
      py+= mSin(speed1+phase[winno][1][h])/(h+1);
/*
      px+=mSin(speed1+phase[winno][0][h]);
      py+=mSin(speed1+phase[winno][1][h]);
*/
      /*      pz+=mSin((h+1)*t+phase[winno][2][h]); */
    }
    px>>=8;py>>=8;
    /* pz>>=9; 
       pzmod = 1;
       (pz/16+0.25); */
      
    /* red */
    n1[winno][num	  ].x=midCX+px;
    n1[winno][num+1*NUMPTS].x=midCX+py;
    n1[winno][num+2*NUMPTS].x=midCX-px;
    n1[winno][num+3*NUMPTS].x=midCX-py;
    n1[winno][num	  ].y=midCY+py;
    n1[winno][num+1*NUMPTS].y=midCY-px;
    n1[winno][num+2*NUMPTS].y=midCY-py;
    n1[winno][num+3*NUMPTS].y=midCY+px;

/*
    nx1[winno][num         ]=( px);
    nx1[winno][num+1*NUMPTS]=( py);
    nx1[winno][num+2*NUMPTS]=(-px);
    nx1[winno][num+3*NUMPTS]=(-py);
    
    ny1[winno][num    ]     =( py);
    ny1[winno][num+1*NUMPTS]=(-px);
    ny1[winno][num+2*NUMPTS]=(-py);
    ny1[winno][num+3*NUMPTS]=( px);
*/
#if THREED    
    /* blue */
    n2[winno][num	  ].x=midCX+px;
    n2[winno][num+1*NUMPTS].x=midCX+py;
    n2[winno][num+2*NUMPTS].x=midCX-px;
    n2[winno][num+3*NUMPTS].x=midCX-py;
    n2[winno][num	  ].y=midCY+py;
    n2[winno][num+1*NUMPTS].y=midCY-px;
    n2[winno][num+2*NUMPTS].y=midCY-py;
    n2[winno][num+3*NUMPTS].y=midCY+px;

/*
    nx2[winno][num    ]     =( px);
    nx2[winno][num+1*NUMPTS]=( py);
    nx2[winno][num+2*NUMPTS]=(-px);
    nx2[winno][num+3*NUMPTS]=(-py);

    ny2[winno][num    ]     =( py);
    ny2[winno][num+1*NUMPTS]=(-px);
    ny2[winno][num+2*NUMPTS]=(-py);
    ny2[winno][num+3*NUMPTS]=( px);
   */
#endif 
  }

  if((options.opt1&1)==1) /* draw lines connecting 'em */
  {
    int i,start,end;

    if(options.multi)my_col= (my_col+1)%numcolors;
   /* draw the ones at the ends (127->0) */
    for(i=0;i<4;i++)  
    {
      start=(NUMPTS-1+(i*NUMPTS));
      end  =(NUMPTS*i);
      
      XDrawLine(display,window[winno],color_gcs[0],
		o1[winno][start].x,
		o1[winno][start].y,
		o1[winno][end].x,
		o1[winno][end].y);
      
      XDrawLine(display,window[winno],color_gcs[my_col],
		n1[winno][start].x,
		n1[winno][start].y,
	        n1[winno][end].x,
		n1[winno][end].y);

#if THREED
      XDrawLine(display,window[winno],color_gcs[0],
		midCX+ox2[winno][start],
		midCY+oy2[winno][start],
		midCX+ox2[winno][end],
		midCY+oy2[winno][end]);
      XDrawLine(display,window[winno],color_gcs[numcolors>>1],
		midCX+nx2[winno][start],
		midCY+ny2[winno][start],
		midCX+nx2[winno][end],
		midCY+ny2[winno][end]);
#endif

    } 
    if(options.multi)
    {
      my_col=loops++;if(loops>=(numcolors-1))loops=0;
    } 
    /* draw (and erase) main points (0-126)->(1-127) */

    /*    for(x=0;x<NUMPTS-1;x++)
    {
    */
      
      for(i=0; i<4; i++)
      {
        if(options.multi) {my_col+=2;if(my_col>=numcolors)my_col=1;}
      /*  fprintf(stderr,"(%ld,%ld)\n",midCX+nx1[x],midCY+ny1[x]); */

	/*
	start=x+(i*NUMPTS);
	end=start+1;
	*/
	if((options.opt1&2)==0){
	  /*
	  XDrawLine(display,window[winno],color_gcs[0],
		    midCX+ox1[winno][start],
		    midCY+oy1[winno][start],
		    midCX+ox1[winno][end],
		    midCY+oy1[winno][end]); 
		    */
	  XDrawLines(display,window[winno],color_gcs[0],
		     &o1[winno][i*NUMPTS],NUMPTS,CoordModeOrigin);
	  
	}
	/*
	XDrawLine(display,window[winno],color_gcs[my_col],
		  midCX+nx1[winno][start],
		  midCY+ny1[winno][start],
		  midCX+nx1[winno][end],
		  midCY+ny1[winno][end]);
	*/

	XDrawLines(display,window[winno],color_gcs[my_col],
		   &n1[winno][i*NUMPTS],NUMPTS,CoordModeOrigin);

	/*	ox1[winno][start]=nx1[winno][start];
		oy1[winno][start]=ny1[winno][start];
	*/
      } 
      /*
	}
	*/
    /* copy fringe points into old */
  /*    for(i=0;i<4;i++)
    {
      start=(NUMPTS-1+(i*NUMPTS));
      end  =(NUMPTS*i);
      ox1[winno][start]=nx1[winno][start];
      oy1[winno][start]=ny1[winno][start];
    }
    */
  }
  else   /* draw the points */
    {
      XDrawPoints(display,window[winno],color_gcs[0],
		  o1[winno],NUMPTS*4,CoordModeOrigin);
      XDrawPoints(display,window[winno],color_gcs[colr],
		  n1[winno],NUMPTS*4,CoordModeOrigin);
      
      /*
      for(x=0;x<4*NUMPTS;x++)
      {
	XDrawPoint(display,window[winno],
		   color_gcs[0],midCX+ox1[winno][x],
		   midCY+oy1[winno][x]);
	XDrawPoint(display,window[winno],
		   color_gcs[colr],midCX+nx1[winno][x],
		   midCY+ny1[winno][x]);
	ox1[winno][x]=nx1[winno][x];
	oy1[winno][x]=ny1[winno][x];
      }
      */
    }

      /* pointer swap, so that o1 is now the set of pts that was
	 just drawn, and n1 is able to be used over */
   temp=o1[winno]; o1[winno]=n1[winno]; n1[winno]=temp;
 
  /* increase all the phases */
  for(y=0;y<3;y++)
  {
    for(x=0;x<options.number;x++)
    {
      phase[winno][y][x] += phase_add[winno][y][x];
    }
  }
}

/*   Like a cross between 'rose' and 'starback'
 *
 */
/*
void
draw_surfer_rosa(int winno)
{
  int i;
  
  draw_rose(winno);
  
  for(i=0;i<4*NUMPTS;i++)
  {
    
    
  }
}
*/
