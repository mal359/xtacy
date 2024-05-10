/*
**  Xtacy (X11 Version)                     -><- HAIL ERIS!
**
**   IN THE BEGINNING	, there was Kaleid
**                                     
**  Original EGA/VGA version of Kaleid (for IBM PCs and compatibles) by:
**    Judson D. McClendon
**    Sun Valley Systems
**    329 37th Court N.E.
**    Birmingham, AL 35215
**    Compuserve: [74415,1003]
**
**  Ported to X11 by Nathan Meyers, nathanm@hp-pcd.hp.com.
**
**  And then, Jer Johnson got hold of it, and poor Kaleid became:
**
** 			Xtacy.
** 
**   (to see what the original Kaleid looked like, run
**    'xtacy -kaleid -norect -colors 16')
** 
**  Pretty Shapes and Colors, Boxes, Bozogons, Qixes (Qices?), wandering
**  modern art , plasma clouds, Gravity Wells, and...ummm...
**  swarm, tag, bezier, and whatever else I added,
**  added by Jeremy Johnson, mpython@gnu.ai.mit.edu
**
**  thanks to Matt Cross (profesor@wpi), for the Tunnel code 
**
**  thanks to Daniel Cabeza Gras for the mixer and cells modes, and for
**  speeding up the Spiral draw
**  
**  thanks to Noah Vawter (shifty@gweep.net) for the new rose code
**
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program; if not, write to the Free Software
**    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <signal.h>
#include <math.h>
#include <sys/stat.h>
#include "vroot.h"  /* for the complete virtual rootwin experience */
#include "trippy.h" /* all the global vars */
#include "lmath.h"
#include "waves.h"

#define VERSION "1.14"

struct foo options;
int *jj;
int startup=0;
int dropachicken=0;
/* extern struct particle **partlst; */
int l,r,t,b;
int x,y;
static double coordx,coordy;
extern void MakeWindow(int,char **);
extern void MakePerfectWindow(int,char **);
extern void makeSubRootWins(int,char **);
extern void makeLockWin(int,char **);
static char *modeStr(int);

void handle_event(XEvent*);
static int scheduler();
static int defaultNumber();
static void exit_mode(int);

int initstuff(int);
void refreshrootquit(int);
void fatalerror(char *, char *);
XEvent event;
Visual *vis;
unsigned int depth;
char *password;
struct timeval *schedule, *intervals; 
int visclass;

int
main(int argc,char **argv)
{
  int i;
  char **o_argv;
  int o_argc;
  XVisualInfo vinfo;
  /* int inloop; */
  
  srndm(time(NULL));
  
/***** set up the structure *****/
  options.bwidth=2;
  options.number=-1;
  options.delayvalue= (-1);
  options.doroot=0;
  options.annoy_tefler = 0; /* Don't Annoy Tefler by Default */
  options.noclear=0;
  options.mono=0;
  options.trell=0;
  options.norect=0;
  options.multi=0;
  options.dynamic_colors=0;
  options.totalrand=0;
  options.mode= (modes)-1;
  options.mirrors=0;
  options.tryfor=NCOLORS;
  options.displayname = NULL;
  options.geomstring = NULL;
  options.palette_filename= NULL;
#ifdef JPEG_SUPPORT
  options.loadfile=NULL;
#endif
  options.windows=1;
  options.rillybig=0;
  options.palette=0; /* original */
  options.numparts=0;
  options.win =0;
/* Real lock mode added by Jeff Magill */
  options.debug = 0;
  options.lock = 0;
#ifdef SYSTEMPASSWORD
  options.inqPassword = 0;
#else
  options.inqPassword = 1;
#endif
  options.allowroot = 0;

  
  share_colors=0;  /* mine! mineminemine! */
  coordx = -0.194;
  coordy =  0.6667;
  o_argc = argc;
  o_argv = argv;
  progname = argv[0];
  
/* go thru the command line arguments, and set the options structure 
 * accordingly
 */
  parse_options(o_argc, o_argv);

  if (options.win !=0)
    options.windows=1;
  if (options.delayvalue == -1)
    options.delayvalue = options.doroot ? 100.0 : 10.0;
  if (options.mode == -1)
  {
    options.mode = (modes)rndm((long)lightning);
    if(options.mode==xstatic)
      options.dynamic_colors=1;
  }
  if (options.number == -1)
    options.number = defaultNumber();

  /* lazy Jer doesn't want to spend time right now fixing the
     rotating shapes modes with multiple windows */
  if(options.mode == rot_shape || options.mode == cube ||
     options.mode == tet       || options.mode == oct)
  {
    if(options.windows>1)
    {
      options.windows=1;
      fprintf(stderr,"Only using one window with %s\n",modeStr(options.mode));
    }
  }
  

  if ((display=XOpenDisplay(options.displayname))==NULL) 
  {
    fprintf(stderr,
	    (options.displayname==NULL) ?
	    "%s: Failed to open display.\n" :
	    "%s: Failed to open display %s.\n",
	    progname,options.displayname);
    exit(1);
  }
  screen = DefaultScreen(display);
  scr = ScreenOfDisplay (display, screen);

  if (options.lock)
  {
    if (options.inqPassword)
    {
      char *fob;

      fob = (char *) getpass ("Enter a key:\0");
      password = (char *) malloc (16 * sizeof (char));
      strcpy (password, fob);
      fob = (char *) getpass ("Again:\0");
      if (strcmp (fob, password))
      {
	fprintf (stderr, "NOPE! NOT GONNA LOCK! \n");
	options.lock = 0;
      }
      else
      {
	fprintf (stderr, "Right-O! Locking your station\n");
	
	/*
	 * for locking, we pretend that the user is running '
	 * -rillybig ' and not in the root
	 */
	options.rillybig = 1;
	options.doroot = 0;
	options.geomstring = (char *) malloc (10 * sizeof (char));
	sprintf (options.geomstring, "%dx%d", DisplayWidth (display, screen),
		 DisplayHeight (display, screen));
	options.windows = 1;
	options.annoy_tefler = 0;	/* don't do the shrinking
					 * window thing */
	XSetScreenSaver(display,0,0,DontPreferBlanking,DefaultExposures);
      }
    }
    else
    {
      /*
       * we will use the user's and root's password.  We don't need to
       * ask what the password is
       */
      
      /*
       * for locking, we pretend that the user is running ' -rillybig '
       * and not in the root
       */
      options.rillybig = 1;
      options.doroot = 0;
      options.geomstring = (char *) malloc (10 * sizeof (char));
      sprintf (options.geomstring, "%dx%d", DisplayWidth (display, screen),
	       DisplayHeight (display, screen));
      options.windows = 1;
      options.annoy_tefler = 0;	/* don't do the shrinking window
				 * thing */
    }
  }


/*
  vis=DefaultVisual(display,screen);
*/ 
  if (!XMatchVisualInfo(display,screen,8,PseudoColor,&vinfo))
    if(!XMatchVisualInfo(display,screen,16,DirectColor,&vinfo))
      if(!XMatchVisualInfo(display,screen,16,TrueColor,&vinfo))
	if(!XMatchVisualInfo(display,screen,24,DirectColor,&vinfo))
	  if(!XMatchVisualInfo(display,screen,24,TrueColor,&vinfo))
	  { 
	    printf("Can't get colors to work with\n");
	    exit(1);
	  }
  vis=vinfo.visual;
  depth = vinfo.depth;

  if (vis==0)
  {
    vis=DefaultVisual(display,screen);
  }

#if defined (__cplusplus) || defined (c_plusplus)
  visclass = vis->c_class;
#else
  visclass = vis->class;
#endif

  /* If vis->class = 0, it's a Static Grey display */
  if(visclass == 0)
  { 
    fprintf(stderr,"Phwee-Hoo.. No Colors, No Xtacy\n");
    exit(1); 
    options.mono=1;
    options.tryfor=2;
    if(options.mode==off)
    {
      fprintf(stderr,"Sorry, 'off' won't work on a monochrome. Dropping you to kaleid\n");
      options.mode=kaleid;
    }
  }
  else if (visclass>PseudoColor)
  {
    fprintf(stderr,"Weird... Lotsa colors.. I'll have to think about this..\n"  );
    if(visclass==DirectColor) fprintf(stderr,"Direct Color(%d entries)..\n",vis->map_entries);
    if(visclass==TrueColor)   fprintf(stderr,"True Color(%d entries)...\n",vis->map_entries);
    
  }
  
  if (options.doroot) 
  {
    if(options.rillybig) /* make one Whopping Huge Window */
    {
      window=(Window *)malloc(1*sizeof(Window));
      window[0]=RootWindow(display,screen);
      nwindows=1;
      options.windows=1;
      /* XSetWindowBackground(display,window[0],options.bgcolor); */
      XClearWindow(display,window[0]);
      XSelectInput(display,window[0],0);
      XSync(display,0);
/*	  fprintf(stderr,"Window[0]=%lx\n ",window[0]); */
      CX=(unsigned int *)calloc(nwindows,sizeof(unsigned int));
      CY=(unsigned int *)calloc(nwindows,sizeof(unsigned int));
    }
    else /* make lotsa little sub-root-windows */
    {
      makeSubRootWins(o_argc,o_argv);
    }
  }    
  else if (options.perfect) /* make a window with 'perfect' colors...
			       grabs the colormap away from everything else */
  {
    window=(Window *)malloc(sizeof(Window)*options.windows);
    MakePerfectWindow(o_argc,o_argv);
    nwindows=options.windows;
  }
  else if(options.lock)
  {
    makeLockWin(o_argc,o_argv);
  }
  else
  {
    window=(Window *)malloc(sizeof(Window)*options.windows);
    if(options.win)
      window[0]=options.win;
    else
      MakeWindow(o_argc,o_argv);
    nwindows=options.windows;
  }
  
  if(options.lock)
  {
    /* grab everything , keyboard, mouse, etc. */
    if (!options.debug)
    {
      makeBlankCursor ();
      XGrabKeyboard(display,window[0],False,
		    GrabModeAsync,GrabModeAsync,CurrentTime);
      XGrabPointer (display, window[0], False,
		    (ButtonPressMask | ButtonReleaseMask |
		     EnterWindowMask | LeaveWindowMask |
		     PointerMotionMask | PointerMotionHintMask |
		     Button1MotionMask | Button2MotionMask |
		     Button3MotionMask | Button4MotionMask |
		     Button5MotionMask | ButtonMotionMask |
		     KeymapStateMask),
		    GrabModeAsync, GrabModeAsync, window[0], mycursor,
		    CurrentTime);
    }
  }
  
  if (initstuff(options.windows)==0)
  {
    fprintf(stderr,"Couldn't set up internal tables..Memory error. Exiting\n");
    exit(1);
  }
  
  math_init();
  wave_init();  
/*  fprintf(stderr,"Init'd %d gobs of stuff\n",options.windows); */
  setup_windows();

  
  if(!options.multi && !options.totalrand &&
     (options.mode==qix4 || options.mode==qix   ||
      options.mode==circ || options.mode==boxes ||
      options.mode==bozo || options.mode==bez  ))
    options.tryfor=12; /* we only need 3, leave the rest for other programs */
/* if(vis->class<=PseudoColor) */
  get_them_colors(); /* allocate colors */
    
  if(!options.multi && (options.mode==qix4 || options.mode==qix ||
			options.mode==circ || options.mode==boxes ||
			options.mode==bozo || options.mode==bez ))
    randomize_color();


#ifdef JPEG_SUPPORT
  if(options.loadfile)
    read_JPEG_file(options.loadfile);
#endif
  
  if (options.doroot)
  {
    /* Catch signals */
    signal(SIGINT,refreshrootquit);
    signal(SIGQUIT,refreshrootquit);
    signal(SIGTERM,refreshrootquit);
  }


  l=0; t=0;
  r=CX[0]; b=CY[0];
   
  
  for (;;)   /* jump in the loop */
  {
    int winno=0;
    int reset=0;
    int cycles=0;
    int dontcare;
    unsigned int udontcare;
    Window wdontcare;
    time_t starttime = time(NULL); 
    if(options.totalrand)
    {
      options.mode=(modes)rndm(lightning);
      options.number=rndm(100);
    }

    
    for(winno=0;winno<options.windows;winno++)
    {
/* set up the min/max for the windows */
      XGetGeometry(display,window[winno],&wdontcare,
		   &dontcare,&dontcare, &CX[winno], &CY[winno], 
		   &udontcare, &udontcare);
/*
  if(options.mode==kaleid)
  {
  CX[winno]>>=1;
l=0; t=0;
      r=CX[winno]; b=CY[winno];
     CY[winno]>>=1;
  }
*/
      M[winno] = max(CX[winno],CY[winno]);
      M[winno] = M[winno] ? M[winno] : 1;
    }
    winno=0;

    if(!startup)
    {
      
      printf("Running mode %s\n",modeStr(options.mode));
      
      if(options.mode==boxes || options.mode==bozo || options.mode==circ ||
	 options.mode==qix || options.mode==qix4 || options.mode==bez)
	/* init the boxes */
      { 
	int i;
	for(i=0;i<options.windows;i++)
	  init_frame(i);
      }
      else if(options.mode==kaleid)
      {
	register int i;
	init_kaleid(options.windows);
	for(i=0;i<options.windows;i++)
	  randomize_kal(i);
      }
      else if(options.mode==blob)
      {
	initBlob(winno);
      }
      else if(options.mode==gravity)
      {
	if(options.numparts==0)
	  options.numparts=1;
	for(i=0;i<options.windows;i++)
	  init_gravity(i);
      }
      else if(options.mode==life)
      {
	for(i=0;i<options.windows;i++)
	  getALife(i);
      }
#if 0   /* not ready for prime time */
      else if (options.mode==oscill)
      {
	init_oscill(winno);
      }
#endif
      else if (options.mode==munch)
      {
	init_munch(winno);
      }
      else if (options.mode==starback)
      {
	for(i=0;i<options.windows;i++)
	  init_galaxy(i);
      }
      else if (options.mode==swarm)
      {
	for(i=0;i<options.windows;i++)
	  init_swarm(i);
      }
      else if (options.mode==tag)
      {
	for(i=0;i<options.windows;i++)
	  init_playground(i);
      }
      else if(options.mode==wandering)
      {
	for(i=0;i<options.windows;i++)
	{
	  /* init the wandering stuff*/
	  x=rndm(CX[i]);
	  y=rndm(CY[i]);
	  HC[i]=rndm(numcolors);
	}
      }
      else if(options.mode==fields)
      {
	for(i=0;i<options.windows;i++)
	  init_field(i);
      }
      else if (options.mode==flush)
      {
	init_flush(winno);
      }
      else if (options.mode==blur)
      {
	init_blur(winno);
      }
      else if(options.mode==rose)
      {
	init_rose();
      }
    }
    
    startup=1;    

/*      printf("Window=%lx\n",window); */
      
    while (reset==0) 
    { 
      if(options.maxcycles!=0)
      {
        if(cycles++>options.maxcycles)
	{
	  int elapse = time(NULL) - starttime;
	  fprintf(stderr,"%d cycles complete in %d seconds \n",options.maxcycles,elapse);
	  exit(0);
	}
      }
      reset=0;
/*	fprintf(stderr,"Drawing Window[%d]\n",winno); */
      switch(options.mode)
      {
	case kaleid:
	{
	  draw_kaleid(winno);
	  break;
	}
        case munch:
	{
	  draw_munch(winno,x++);
	  break;
	}
	case flush:
	{
	  draw_flush(winno);
	  break;
	}
	case blur:
	{
	  draw_blur(winno);
	  break;
	}
#ifdef ALPHABETA
	case net:
	{
	  draw_net(winno);
	  break;
	}
	case xload:
	{
	  static time_t ti=0;

	  if((time(NULL)-ti) > 5)
	  {
	    draw_xload(winno);
	    ti = time(NULL);
	  }
	  else
	  {
	    rotate_colors();
	  }
	  break;
	}
#endif
	case test:
	{
	  if (jj[winno]==0)
	  {
	    draw_test(winno);
	    jj[winno]=1;
	  }
	  break;
	}
	case blob:
	{
	  moveBlob(winno);
	  break;
	}
	case off:
	{
	  if (jj[winno]==0)
	  {
	    draw_off(winno);
	    jj[winno]=1;
	    XFlush(display);  /* display it right now */
	  }
	  break;
	}
	case maxh:   /* same thing now */
	case cube:
	case tet:
	case oct:
	case rot_shape:
	{
	  rotate_shape(winno);
	  break;
	}
	case wandering:
	{
	  wander(winno,&x,&y);
	  rotate_colors();
	  break;
	}
	case lightning:
	{
	  if(rndm(100)<options.number)
	  {
	    if(jj[winno]==0)
            {
	      lightning_strike(winno);
	      jj[winno]=1;
            }
	    else
	    {
	      de_strike(winno);
	      jj[winno]=0;
	    }
	  } 
	  break; 
	}
	case rose:
	{
	  draw_rose(winno);
	  break;
	}
	case funky:
	{
	  draw_funky(winno);
          if((jj[winno]++ % 50) ==0) 
	    rotate_colors();
	  break;
	}
	case life:
	{
	  drawLife(winno);
	  break;
	}
	case starback:
	{
	  draw_stars(winno);
	  break;
	}
	case tag:
	{
	  draw_playground(winno);
	  break;
	}
	case swarm:
	{
	  draw_swarm(winno);
	  break;
	}
	case gravity:
	{
	  draw_gravity(winno);
	  break;
	}
#if 0 
	case oscill:
	{
	  draw_oscill(winno);
	  break;
	}
#endif
        case lunch:
	{
	  draw_lunch(winno,&jj[winno]);
	  break;
	}
	case spheres:
	{
	  draw_sphere(winno);
	  break;
	}
/* these are the palette shifty ones */
	case dline:
        case dcurve:
	case sunrise:
	case sunflower:
	case clover:
	case tunnel:
	case mixer:
	case cells:
	case mandel:
	case julia:
	case plasma:
	case dogplasma:
	case spiral:
	case newton:
	case ripple:
	case xstatic:
	case fields:
	case taffy:
	case waves:
	case radial:
	case fracrect:
	{	      
	  if(jj[winno])
	  {
	    rotate_colors();
	    startup=0;
	  }
	  else
	  {
	    switch (options.mode)
	    {
	      case sunflower:
	      {
		draw_sunflower(winno);
		jj[winno]=1;
		break;
	      }
	      case dline:
	      {
		draw_dline(winno);
		jj[winno]=1; 
		break;
	      }
	      case dcurve:
	      {
		draw_dcurve(winno);
		jj[winno]=1; 
		break;
	      }
	      case waves:
	      {	  
		if (rndm(20)<10)
		{
		  for(i=0;i<CY[winno];i=i+1)
		  {
		    r=r+rndm(20)-10;
		    wave_drawh(winno,rndm(2),0,i,CX[winno],0,r,
			       30+rndm(options.number),0);
		  }
		}
		else
		{
		  for(i=0;i<CX[winno];i=i+1)
		  {
		    r=r+rndm(20)-10;
		    wave_drawv(winno,rndm(2),i,0,CY[winno],0,r,
			       30+rndm(options.number),0);
		  }
		}
		jj[winno]=1;
		break;
	      }
	      case radial:
	      {
		wave_radial(winno,0,rndm(CX[winno]),rndm(CY[winno]),M[winno],
			    0,25,50);
		jj[winno]=1;
		break;
	      }
	      case fracrect:
	      {
		draw_fracrect(winno,0,0,CX[winno],CY[winno],
			      rndm(numcolors),1);
		jj[winno]=1;
		startup=1;
		break;
	      }
	      case sunrise:
	      {
		draw_sunrise(winno);
		jj[winno]=1;
		startup=1;
		break;
	      }
	      case taffy:
	      {
		draw_taffymaker(winno);
		jj[winno]=1;
		startup=1;
		break;
	      }
	      case clover:
	      {
		draw_clover(winno);
		jj[winno]=1;		  
		startup=1;
		break;
	      }
	      case tunnel:
	      {
		draw_tunnel(winno);
		jj[winno]=1;		  
		startup=1;
		break;
	      }
	      case mixer:
	      {
		draw_mixer(winno);
		jj[winno]=1;
		startup=1;
		break;
	      }
	      case cells:
	      {
		draw_cells (winno);
		jj[winno]=1;
		startup=1;
		break;
	      }
	      case xstatic:
	      {
		draw_static(winno);
		jj[winno]=1;
		startup=1;
		break;
	      }
	      case mandel:
	      {
		draw_mandel(l,t,r,b,winno);
		jj[winno]=1;
		startup=1;
		break;
	      }
	      case julia:
	      {
		draw_julia(winno,coordx,coordy);
		jj[winno]=1;
		startup=1;
		break;
	      }
	      case fields:
	      {
		draw_field(winno);
		jj[winno]=1;
		break;
	      }
	      case ripple:
	      {
		if(options.number>10)
		{
		  options.number=10;
		}
		draw_ripple(winno);
		jj[winno]=1;
		break;
	      }
	      case plasma:
	      {
		draw_plasma(winno);
		startup=1;		  
		jj[winno]=1;
		break;
	      }
	      case dogplasma:
	      {
		draw_dogplasma(winno);
		startup=1;
		jj[winno]=1;
		break;
	      }
	      case spiral:
	      {
		draw_spiral(winno);
		startup=1;
		jj[winno]=1;
		break;
	      }
	      case newton:
	      {
		draw_newton(winno);
		startup=1;
		jj[winno]=1;
		break;
	      }
	      default:
	      {
		fprintf(stderr,"Hmm.. must've taken a left turn at Albequerque\n");
		exit(-1);
	      }
	    }
	  }
	  break;
	}
/* these are the bouncy ones */
	case bez:
	case bozo:
	case circ:
	case qix:
	case qix4:
	case boxes:
	{ 
	  bounce(winno);	 /* move the shape */
	  switch(options.mode)
	  {
	    case bozo:
	    {
	      draw_bozo(winno);
	      break;
	    }
	    case circ:
	    {
	      draw_circ(winno);
	      break;
	    }
	    case qix:
	    {
	      draw_qix(winno); break;
	    }
	    case qix4:
	    {
	      draw_qix4(winno); break;
	    }
	    case boxes:
	    {
	      draw_boxes(winno); break;
	    }
	    case bez:
	    default:
	    {
	      draw_bez(winno);
	      break;
	    }
	  }
	  move_frame(winno);
		
	  break;
	}
      }

/*      XFlush(display); */
      if(options.mode==kaleid||options.mode==test)
      {
	if (!rndm(500L)&&!(options.doroot)&&(options.mode!=test)) 
	  XClearWindow(display,window[winno]);
	if (options.dynamic_colors && !rndm((long)(800/numcolors)))
	  randomize_color();
      }
      else
      {
	if (!options.mono) 
	{
	  if(!options.multi && !rndm(500L))
	    HC[winno]=rndm(numcolors);
	  if(options.multi && (options.mode==wandering))
	    HC[winno]=(HC[winno]+1)%numcolors;
	}
      }
      winno=scheduler();

/* if any event is waiting, go try to handle it */
      if (XCheckMaskEvent(display,~0L,&event)==True) 
	handle_event(&event);
      
      if((options.totalrand&&(rndm(10000L)==2317))||dropachicken)
      {
	exit_mode(winno);
	if(options.totalrand)
	{
	  options.mode=(modes)rndm(lightning);
	  options.dynamic_colors=rndm(1);
	}
	dropachicken=0;
	jj[winno]=0;
	reset=1;
	startup=0;
	break; /* drop out of the while(nvisible) loop */
      }
    }
    handle_event(&event);
  }
  
}

void
exit_mode(int winno)
{
  switch(options.mode)
  {
    case kaleid:      { exit_kaleid(); break; }
    case boxes:	      { exit_frame(winno); break;}
    case bozo:	      { exit_bozo(winno); break;}
    case test:	      { exit_test(); break;}
    case circ: 	      { exit_circ(winno); break;}
    case wandering:   { exit_wandering(); break;}
    case qix: 	      { exit_qix(winno); break;}
    case qix4: 	      { exit_qix4(winno); break;}
    case clover:      { exit_clover(); break;}
    case tunnel:      { exit_tunnel(); break;}
    case plasma:      { exit_plasma(); break;}
    case rose: 	      { exit_rose(); break;}
    case spiral:      { exit_spiral(); break;}
    case mandel:      { exit_mandel(); break;}
    case julia:	      { exit_julia(); break;}
    case gravity:     { exit_gravity(); break;}
    case newton:      { exit_newton(); break;}
    case off:	      { exit_off(); break;}
    case spheres:     { exit_sphere(); break;}
    case funky:	      { exit_funky(); break;}
    case ripple:      { exit_ripple(); break;}
    case life:	      { exit_life(); break;}
    case tag:         { exit_tag(); break;}
    case xstatic:     { exit_xstatic(); break;}
    case maxh:	      { exit_maxh(); break;}
    case fields:      { exit_fields(); break;}
    case starback:    { exit_starback(); break;}
    case mixer:       { exit_mixer(); break;}
    case cells:	      { exit_cells(); break;}
    case taffy:	      { exit_taffy(); break;}
    case bez:	      { exit_bez(winno); break;}
    case swarm:	      { exit_swarm(); break;}
    case dogplasma:   { exit_dogplasma(); break;}
    case cube:        { exit_cube(); break;}
    case oct:         { exit_oct(); break;}
    case tet:         { exit_tet(); break;}
    case flush:	      { exit_flush(); break;}
    case blur:	      { exit_blur(); break;}
    case rot_shape:   { exit_shape(); break;}
    case lightning:   { exit_lightning(); break;}
#if 0
    case xload:	      { exit_xload(); break;}
#endif
    case sunrise:     { exit_sunrise(); break;}
    case blob:	      { exit_blob(); break;}
    case waves:	      { exit_waves(); break;}
    case fracrect:    { exit_fracrect(); break;}
    case radial:      { exit_radial(); break;}
#if 0
    case oscill:      { exit_oscill(); break;}
#endif
    case dline:	      { exit_dline(); break;}
    case dcurve:      { exit_dcurve(); break;}
#ifdef ALPHABETA
    case net:         { exit_net(); break;}
#endif
    case munch:       { exit_munch(); break;}
    case lunch:       { exit_lunch();  break;}
    default: 
    {
      fprintf(stderr, "No epilog for mode %s(%d)\n",modeStr(options.mode),options.mode);
      exit(1);
    }
  }
}

void
parse_options(int argc, char **argv)
{
  while (--argc>0) 
  {
    char *option = (*++argv);
    if (!strcmp(option,"-display")) 
    {
      if (--argc==0)
	usage();
      options.displayname = (*++argv);
    }
    else if (strchr(option,':')) 
    {
      options.displayname = option;
    }
    else if (!strncmp(option,"-annoy",6))
    {
      options.annoy_tefler=1; /* do that Zoomy close window thang */
    }
    else if (!strncmp(option,"-bez",4))
    {
      options.mode = bez;
    }
/*
  else if (!strcmp(option,"-bg")) 
  {
  if (--argc==0) 
  usage();
  options.background = (*++argv);
  }
  else if (!strcmp(option,"-bd")) 
  {
  if (--argc==0)
  usage();
  options.border = (*++argv);
  }
*/
    else if (!strcmp(option,"-blob"))
    {
      options.mode = blob;
    }
    else if (!strcmp(option,"-blur"))
    {
      options.mode=blur;
    }
    else if (!strcmp(option,"-boxes"))
    {
      options.mode=boxes;
    }
    else if (!strcmp(option,"-bozo"))
    {
      options.mode=bozo;
    }
/*
  else if (!strcmp(option,"-bw")) 
  {
  if (--argc==0)
  usage();
  options.bwidth = atoi(*++argv);
  if (options.bwidth<0)
  options.bwidth = 0;
  }
*/
    else if (!strcmp(option,"-cells"))
    {
      options.mode=cells;
    }
    else if (!strncmp(option,"-circ",5))
    {
      options.mode = circ;
    }
    else if (!strcmp(option,"-clover"))
    {
      options.mode = clover;
    }
    else if (!strcmp(option,"-colors"))
    {
      if (--argc==0)
	usage();
      options.tryfor = atoi(*++argv);
      if (options.tryfor<0)
	options.tryfor=2;
    }
    else if (!strncmp(option,"-coord",6))
    {
      if (--argc==0)
	usage();
      coordx=atof(*++argv);
      if (--argc==0)
	usage();
      coordy=atof(*++argv);
      /*printf("%f  %f\n", coordx, coordy);*/
    } 
    else if (!strcmp(option,"-cube"))
    {
      options.mode=cube;
    }
    else if (!strcmp(option,"-cycles"))
    {
      if (--argc==0)
	usage();
      options.maxcycles = atoi(*++argv);
      if (options.maxcycles<0)
	options.maxcycles=0;

    }
    else if (!strcmp(option,"-delay")) 
    {
      if (--argc==0)
	usage();
      options.delayvalue = atof(*++argv);
      if (options.delayvalue<0)
	options.delayvalue = 0.0;	    
    }
    else if (!strncmp(option,"-dcurve",6))
    {
      options.mode=dcurve;
    }
    else if (!strncmp(option,"-dline",6))
    {
      options.mode=dline;
    }
    else if (!strncmp(option,"-dog",4))
    {
      options.mode=dogplasma;
    }
    else if (!strcmp(option,"-few"))
    {
      options.tryfor=18;
    }
    else if (!strncmp(option,"-field",5))
    {
      options.mode=fields;
    }
    else if (!strncmp(option,"-file",5))
    {
#ifdef JPEG_SUPPORT
      if (--argc==0)
	usage();
      options.loadfile = (*++argv);      
#endif
      
    }
    else if (!strcmp(option,"-flush"))
    {
      options.mode=flush;
    }
    else if (!strncmp(option,"-frac",5))
    {
      options.mode=fracrect;
    }
    else if (!strcmp(option,"-funky"))
    {
      options.mode=funky;
    }
    else if (!strcmp(option,"-several"))
    {
      options.tryfor=18*4;
    }
    else if (!strcmp(option,"-lotsa"))
    {
      options.tryfor=NCOLORS;
    }
    else if (!strcmp(option,"-geometry")) 
    {
      if (--argc==0)
	usage();
      options.geomstring = (*++argv);
    }
    else if (*option=='=') 
    {
      options.geomstring = option;
    }
    else if (!strcmp(option,"-gravity"))
    {
      options.mode=gravity;
      if(options.numparts==0)
	options.numparts=1;
      options.delayvalue = 0;
    }
    else if (!strcmp(option,"-julia"))
    {
      options.mode=julia;
    }
    else if (!strcmp(option,"-kaleid"))
    {
      options.mode=kaleid;
      if(options.mirrors==0)
	options.mirrors=7;
    }
    else if (!strcmp(option,"-life"))
    {
      options.mode=life;
    }
    else if (!strncmp(option,"-light",6))
    {
      options.mode=lightning;
    }
    else if(!strcmp(option,"-lock"))
    {
      options.lock=1;
    }
    else if (!strcmp(option,"-lunch"))
    {
      options.mode = lunch;
    }
    else if (!strcmp(option,"-mandel"))
    {
      options.mode= mandel;
    }
    else if (!strcmp(option,"-max"))
    {
      options.mode = maxh; /* same as cube */
    }
    else if (!strncmp(option,"-mirror",7))
    {
      if (--argc==0)
	usage();
      options.mirrors = atoi(*++argv);
    }
    else if (!strcmp(option,"-mixer"))
    {
      options.mode=mixer;
    }
    else if (!strncmp(option,"-mono",5)) 
    {
      options.mono=1;
    }
    else if (!strcmp(option,"-multi")) 
    {	
      options.multi=1;
    }
    else if (!strcmp(option,"-munch"))
    {
      options.mode=munch;
    }
#ifdef ALPHABETA
    else if (!strcmp(option,"-net"))
    {
      options.mode=net;
    }
#endif
    else if (!strcmp(option,"-newton"))
    {
      options.mode=newton;
    }
    else if (!strcmp(option,"-noclear"))
    {
      options.noclear=1;
    }
    else if (!strncmp(option,"-num",4))
    {
      if (--argc==0)
	usage();
      options.number=atoi(*++argv);
      if(options.number<0)
	options.number=0;
    }
    else if (!strcmp(option,"-norect"))
    {
      options.norect=1;
    }
    else if (!strcmp(option,"-oct"))
    {
      options.mode=oct;
    }
    else if (!strcmp(option,"-off"))
    {
      options.mode=off;
      options.tryfor=30;
    }
    else if (!strncmp(option,"-opt",4))
    {
      if (--argc==0)
	usage();
      options.opt1=atoi(*++argv);
      if(options.opt1<0)
	options.opt1=0;
    }
#if 0
    else if (!strncmp(option,"-oscill",7))
    {
      FILE *han;
      if((han=fopen("/dev/dsp","r"))!=NULL)
      {
        options.mode = oscill;
	fclose(han);
      }
      else
      {
	fprintf(stderr,"Sound device not available.  No Silly Scope\n");
      }
    }
#endif
    else if (!strncmp(option,"-pal",4))
    {
      struct stat st;
      
      if(--argc==0)
	usage();
      options.palette_filename = *++argv;
      if(stat(options.palette_filename,&st)==-1)  /* nope, not a local file */
      {
	const char path[]="/usr/local/lib/xtacy";
	char *fullpath=(char*)malloc((strlen(path)+
				      strlen(options.palette_filename)+1)*
				     sizeof(char));
	sprintf(fullpath,"%s/%s",path,options.palette_filename);
/*	fprintf(stderr,"Fullpath=%s\n",fullpath); */
	if(stat(fullpath,&st)==-1)  /* see if the file exists */
	{
	  options.palette=atoi(options.palette_filename);
	  free(fullpath);
	}
	else
	{
	  options.palette_filename = fullpath;
	  options.palette = 24; /* 24 is the read from file palette */ 
	}
      }
      else
	options.palette = 24; /* 24 is the read from file palette */ 
    }
    else if(!strncmp(option,"-part",4))
    {
      if(--argc==0)
	usage();
      options.numparts=atoi(*++argv);
      if(options.numparts<1)
	options.numparts=1;
    }
    else if (!strcmp(option,"-perfect"))
    {
      options.perfect=1;
      options.tryfor=1024;
    }		  
    else if (!strcmp(option,"-plasma"))
    {
      options.mode=plasma;
    }
    else if (!strcmp(option,"-qix"))
    {
      options.mode=qix;
    }
    else if (!strcmp(option,"-qix4"))
    {
      options.mode=qix4;
    }
    else if ((!strcmp(option,"-r")) || (!strcmp(option,"-root")))
    {
      options.doroot = 1;
    }
    else if (!strcmp(option,"-radial"))
    {
      options.mode=radial;
    }
    else if (!strncmp(option,"-rand",5)) 
    {
      options.dynamic_colors = 1;
    }
    else if (!strncmp(option,"-rilly",6))
    {
      options.geomstring=(char *)malloc(10*sizeof(char));
      strcpy(options.geomstring,"1024x860");
      options.rillybig=1;
    }
    else if (!strcmp(option,"-ripple"))
    {
      options.mode=ripple;
    }
    else if (!strcmp(option,"-rose"))
    {
      options.mode=rose;
    }
    else if (!strcmp(option,"-shape"))
    {
      options.mode=rot_shape;
    }
    else if (!strcmp(option,"-sphere"))
    {
      options.mode=spheres;
    }
    else if (!strcmp(option,"-spiral"))
    {
      options.mode=spiral;
    }
    else if (!strncmp(option,"-star",5))
    {
      options.mode=starback;
    }
    else if (!strcmp(option,"-static"))
    {
      options.mode=xstatic;
      options.dynamic_colors=1; /* random colors */
    }
    else if (!strcmp(option,"-sunflower"))
    {
      options.mode=sunflower;
    }
    else if (!strcmp(option,"-sunrise"))
    {
      options.mode=sunrise;
    }
    else if(!strcmp(option,"-swarm"))
    {
      options.mode=swarm;
    }
    else if (!strcmp(option,"-taffy"))
    {
      options.mode=taffy;
    }
    else if(!strcmp(option,"-tag"))
    {
      options.mode=tag;
    }
    else if (!strcmp(option,"-test"))
    {
      options.mode=test;
    }
    else if (!strcmp(option,"-tet"))
    {
      options.mode=tet;
    }
    else if (!strncmp(option,"-total",6))
    {
      options.totalrand=1;
    }
    else if(!strcmp(option,"-trell"))
    {
      options.trell=1;
    }
    else if(!strcmp(option,"-tunnel"))
    {
      options.mode=tunnel;
    }
    else if (!strcmp(option,"-wander"))
    {
      options.mode=wandering;
      options.delayvalue=.1;
    }
    else if (!strcmp(option,"-waves"))
    {
      options.mode=waves;
    }
    /*
      else if(!strcmp(option,"-xload"))
      {
      options.mode=xload;
      }
    */
    else if (!strcmp (option, "-debug"))
    {
      printf("debugging mode won't lock the screen.\n");
      options.debug = 1;
    }
    else if (!strcmp (option, "-inq"))
    {
      options.inqPassword = 1;
    }
    else if (!strcmp (option, "-allowroot"))
    {
      options.allowroot = 1;
    }
    else if(!strcmp(option,"-win"))
    {
      if(--argc==0)
	usage();
      options.win = (Window) strtol (*++argv, NULL, 16);

    }
    else if(!strcmp(option,"-windows"))
    {
      if(--argc==0)
	usage();
      options.windows=atoi(*++argv);
      if(options.windows<1)
	options.windows=1;
    }
    else usage();
  }
}

void usage()
{
  fprintf(stderr,"Xtacy Ver. %s by Jer Johnson (jer@gweep.net)\n",VERSION);
  fprintf(stderr,"Usage: %s [-lotsa flags]\n",progname);
  fprintf(stderr," Used everywhere: [-annoy] [-delay <msec>]\n");
  fprintf(stderr,"\t[-cycles <numcycles>] [-display <displayname>] [-geometry <geomtry>]\n\t[-lock] [-r|root] [-rillybig] [-windows <number>]\n");
  fprintf(stderr, " Color Options: [-mono] [-multi] [-few] [-several] [-colors <num>]");
  fprintf(stderr, "\n\t[-rand|randomcolor] [-perfect] [-pal {0|1|2|3|4|5}]\n");
  fprintf(stderr, " Neat Effects: [-bez] [-blur] [-boxes] [-bozo] [-cells] [-circ] [-clover]\n\t[-cube] [-dcurve] [-dline] [-dog] [-field] [-flush] [-fracrect]\n\t[-funky] [-gravity] [-julia]");
  fprintf(stderr, " [-life] [-lunch] [-mandel] [-max]\n\t[-mixer] [-off] [-plasma] [-qix] [-qix4] [-spiral] [-star] [-static]\n\t[-sunrise] [-swarm] [-taffy] [-test] [-tunnel] [-wander]\n");
  fprintf(stderr, " With kaleid: [-norect]\n");
  fprintf(stderr, " With bez,boxes,bozo,circ,qix,qix4:[-number <number>]\n");
  fprintf(stderr, " With taffy,mixer: [-number <width of bands>]\n");
  fprintf(stderr, " With cells: [-number <number of rows>]\n");
  fprintf(stderr, " With life: [-number <density of cells>]\n");
  fprintf(stderr, " With mandel,julia: [-number <num of iterations>\n");
  fprintf(stderr, " With dogplasma,plasma: [-trell] [-number <number>] determines graniness.\n");
  fprintf(stderr, " With gravity: [-number <num Wells>] [-part <num Particles>]\n");
  fprintf(stderr, " With fields: [-number <num points>]\n");
  fprintf(stderr, " With funky: [-number <how funky>]\n");
  
  putc('\n',stderr);
  exit(1);
}

void
fatalerror(char *s1, char *s2)
{
  XEvent event;
  
  fprintf(stderr,"%s: ",progname);
  fprintf(stderr,s1,s2);
  if (XCheckMaskEvent(display,~0L,&event)==True) 
    handle_event(&event);  
  putc('\n',stderr);
  exit(1);
}


int scheduler()
{
  struct timeval currtime, *nextalarm, *alarmindex;
  struct timezone tzp;
  int i;

  /* Get current time */
  (void)gettimeofday(&currtime, &tzp);

  /* Find earliest alarm due */
  alarmindex = nextalarm = schedule;
  for (i=1; i<options.windows; i++) 
  {
    if (visible[++alarmindex - schedule] &&
	( alarmindex->tv_sec < nextalarm->tv_sec ||
	  (alarmindex->tv_sec == nextalarm->tv_sec &&
	   alarmindex->tv_usec < nextalarm->tv_usec)))
      nextalarm = alarmindex;
  }

  /* If the next alarm is not past due, sleep until it comes due */
  if (currtime.tv_sec < nextalarm->tv_sec ||
      (currtime.tv_sec == nextalarm->tv_sec &&
       currtime.tv_usec < nextalarm->tv_usec)) 
  {
    struct timeval timeout;
    int readfds;
    int fd=ConnectionNumber(display);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    timeout.tv_sec = nextalarm->tv_sec - currtime.tv_sec;
    timeout.tv_usec = nextalarm->tv_usec - currtime.tv_usec;
    if (timeout.tv_usec < 0) 
    {
      timeout.tv_sec -= 1L;
      timeout.tv_usec += 1000000L;
    }
      
    readfds = 1<<fd; 
    (void)select(fd+1,(fd_set *) &readfds, NULL, NULL, &timeout);

    /* Recompute current time */
    (void)gettimeofday(&currtime, &tzp); 
  }

  /* Set next alarm to current time + interval */
  nextalarm->tv_sec = currtime.tv_sec+intervals[nextalarm-schedule].tv_sec;
  nextalarm->tv_usec = currtime.tv_usec+intervals[nextalarm-schedule].tv_usec;
  if (nextalarm->tv_usec >= 1000000) 
  {
    nextalarm->tv_sec += 1;
    nextalarm->tv_usec -= 1000000;
  }

  /* Return index of next alarm */
  return nextalarm-schedule;
}


int
initstuff(int nwin)
{
  if(!options.doroot)
  {
    if ((CX=(unsigned int *)calloc(nwin,sizeof(unsigned int))) == NULL)
      return 0;
    if ((CY=(unsigned int *)calloc(nwin,sizeof(unsigned int))) == NULL)
      return 0;
  }
  
  if ((M=(unsigned int *)calloc(nwin,sizeof(unsigned int))) == NULL)
    return 0;
  if ((jj=(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
  if ((visible=(int *)calloc(nwin,sizeof(int))) == NULL) return 0;
  if ((schedule=(struct timeval *)calloc(nwin,sizeof(struct timeval))) == NULL)
    return 0;
  if ((intervals=(struct timeval *)calloc(nwin,sizeof(struct timeval))) == NULL)
    return 0;
  return 1;
}


int
defaultNumber ()
{
  switch (options.mode)
  {
    case sunrise:
      return 256;
    case bez:
    case boxes:
    case bozo:
    case circ:
    case qix:
    case qix4:
    case fracrect:
      return 10;
    case fields:
      return 7;
    case funky:
      return rndm(3);
    case gravity:
    case rose:
      return 2;
    case julia:
    case mandel:
    case newton:
    case plasma:
    case waves:
      return 10;
    case life:
    case starback:
    case swarm:
    case tag:
      return 60;
    case cells:
      return 5;
    case maxh:
      return 12;
    case radial:
    case clover:
    case cube:
    case dogplasma:
    case kaleid:
    case mixer:
    case off:
    case ripple:
    case spheres:
    case spiral:
    case taffy:
    case test:
    case tunnel:
    case wandering:
    default:
      return 1;
  }
}

char*
modeStr(int mode)
{
  switch(mode)
  {
    case kaleid:      return "kaleid";
    case boxes:	      return "boxes";
    case bozo:	      return "bozo";
    case test:	      return "test";
    case circ: 	      return "circle";
    case wandering:   return "wandering";
    case qix: 	      return "qix";
    case qix4: 	      return "qix4";
    case clover:      return "clover";
    case tunnel:      return "tunnel";
    case plasma:      return "plasma";
    case rose: 	      return "rose";
    case spiral:      return "spiral";
    case mandel:      return "mandelbrot";
    case julia:	      return "julia";
    case gravity:     return "gravity";
    case newton:      return "newton";
    case off:	      return "Off the Air";
    case spheres:     return "spheres";
    case funky:	      return "funky";
    case ripple:      return "ripple";
    case life:	      return "life";
    case tag:         return "tag";
    case xstatic:     return "Xstatic!";
    case maxh:	      return "MaxHeadroom";
    case fields:      return "E-fields";
    case starback:    return "starback";
    case mixer:       return "mixer";
    case cells:	      return "cells";
    case taffy:	      return "taffy";
    case bez:	      return "Bezier curve";
    case munch:	      return "Munch";
    case sunflower:   return "sunflower";
    case swarm:	      return "swarm";
    case dogplasma:   return "dog plasma";
    case cube:        return "cube";
    case oct:         return "octahedron";
    case tet:         return "tetrahedron";
    case lightning:   return "lightning";
#ifdef ALPHABETA
    case net:	      return "Net";
    case xload:	      return "xload";
    case oscill:      return "Silly Scope";
#endif
    case blob:	      return "blob";
    case flush:	      return "flush";
    case blur:	      return "blur";
    case waves:	      return "waves";
    case rot_shape:   return "rotating shapes";
    case fracrect:    return "Fracrect";
    case radial:      return "radial";
    case sunrise:     return "sunrise";
    case dline:	      return "D-line";
    case dcurve:      return "D-curve";
    case lunch:       return "Lunch";
    default:          return "Idunno";
  }
}
