/*
 *  XGuts... hiding some of the X details
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "trippy.h"
#include "vroot.h"
#include "password.h"

extern int *jj,startup,dropachicken;
extern char *password;
extern Visual *vis;
extern int l,r,t,b; 
static void maybe_unlock(XEvent*);
static XWindowAttributes win_att;
static XSetWindowAttributes s_win_att;
extern struct particle **partlst;
extern int x,y;
extern struct timeval *schedule, *intervals;

#define PASSLEN 20

void
maybe_unlock (XEvent * event)
{
  Window          textWin;
  char            checkme[80], c;
  XGCValues       values;
  int             charIndex = 0, len = 0, i = 0, done=False;
  char            keystr[PASSLEN];

  checkme[0] = 0;

/* create a window to take in the text */
  textWin = XCreateSimpleWindow (display, window[0],
				 200, 50, 200, 50,
				 options.bwidth, options.bdcolor,
				 options.bgcolor);
  XSelectInput (display, textWin, KeyPressMask);
  XMapWindow (display, textWin);
  XRaiseWindow (display, textWin);

  values.foreground = StrColor ("white", WhitePixel (display, screen));
  XChangeGC (display, color_gcs[1], GCForeground, &values);
  XDrawString (display, textWin, color_gcs[1], 5, 30,
	       options.inqPassword ? "Enter your key:" : "Enter Password:",16);
  XFlush (display);

  /* grab the text. compare it to the password */
  while (done==False)
  {
    if (XCheckMaskEvent (display, ~0L, event) == True)
    {
      switch (event->type)
      {
	case KeyPress:
	  len = XLookupString ((XKeyEvent*)event, keystr, PASSLEN, NULL, NULL);
	  for (i = 0; i < len; i++)
	  {
	    c = keystr[i];
	    switch (c)
	    {
	      case 8:	/* ^H */
	      case 127:	/* DEL */
		if (charIndex > 0)
		  charIndex--;
		break;
	      case 10:	/* ^J */
	      case 13:	/* ^M */
		checkme[charIndex] = '\0';
		done=True;
		break;
	      case 21:	/* ^U */
		charIndex = 0;
		break;
	      default:
		checkme[charIndex] = c;
		if (charIndex < PASSLEN - 1)
		  charIndex++;
		else
		  XSync (display, True);	/* flush input buffer */
		break;
	    }
	  }
	  break;
	case ButtonPress:
	  done=True;
	  break;
	default:
	  break;
      }
    }
  }

  /* if they're equal... kill the 2 windows */
  if ((!options.inqPassword && (matchesPassword(checkme, options.allowroot)))||
      (options.inqPassword && (!strcmp(checkme,password))))
  {
    XDestroyWindow (display, textWin);
    if (!options.debug)
    {
      XUngrabKeyboard (display, CurrentTime);
      XUngrabPointer (display, CurrentTime);
    }
    XFreeColors (display, colmap,(unsigned long*) colors[0], numcolors, 1);
    XFreeColors (display, colmap,(unsigned long*)  colors[1], numcolors, 1);
    XFreeColors (display, colmap,(unsigned long*)  colors[2], numcolors, 1);
    exit (0);
  }
  else
  {
    XDestroyWindow (display, textWin);
    return;
  }
}


void
MakeWindow(int o_argc,char **o_argv)
{
  XSizeHints size_hints;
  int i;
  char *basename;
  if ((basename=strrchr(progname,'/'))!=NULL) 
    basename++;
  else 
    basename=progname;
  
/* top corner of the window */
  size_hints.x = 0;
  size_hints.y = 0;

  if(options.mode==test)
  {
    size_hints.width = 800;
    size_hints.height = 400;
  }
  else if(options.mode==mandel)
  {
    size_hints.width = 600;
    size_hints.height = 600;
  }
  else
  {
    size_hints.height = 400;
    size_hints.width = 400;
  }

  size_hints.flags = PPosition | PSize;
  
/* if there's a geometry string, parse it */
  if (options.geomstring!=NULL) 
  {
    int result;
    result = XParseGeometry(options.geomstring,&size_hints.x,
			    &size_hints.y,(unsigned int*)&size_hints.width,
			    (unsigned int *)&size_hints.height);
    if (result & XNegative)
      size_hints.x += DisplayWidth(display,screen)
	- size_hints.width - options.bwidth*2;
    if (result & YNegative)
      size_hints.y += DisplayHeight(display,screen)
	- size_hints.height - options.bwidth*2;
    if (result & XValue || result & YValue) 
    {
      size_hints.flags |= USPosition;
      size_hints.flags &= ~PPosition;
    }
    if (result & WidthValue || result & HeightValue) 
    {
      size_hints.flags |= USSize;
      size_hints.flags &= ~PSize;
    }
  }
  for(i=0;i<options.windows;i++)
  {
    window[i] = XCreateSimpleWindow(display,RootWindow(display,screen),
				    size_hints.x,size_hints.y,
				    size_hints.width,size_hints.height,
				    options.bwidth,options.bdcolor,
				    options.bgcolor);
    XSetStandardProperties(display,window[i],"Xtacy",basename,
			   None,o_argv,o_argc,&size_hints);
    XSelectInput(display,window[i],
		 StructureNotifyMask|VisibilityChangeMask|
		 ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    XRaiseWindow (display, window[i]);
    XMapWindow(display,window[i]);
    XFlush(display);
/*      fprintf(stderr, "Window= %lx\n",window[i]); */
  }
  return;
}

void
MakePerfectWindow(int o_argc, char **o_argv)
{
  XSizeHints size_hints;
  unsigned long vmask=0;
  XSetWindowAttributes xswat;
  XVisualInfo *vis_info;
  int number,i;
  char *basename;
  if ((basename=strrchr(progname,'/'))!=NULL) 
    basename++;
  else 
    basename=progname;
  size_hints.x = 0;
  size_hints.y = 0;
  if(options.mode==test)
  {
    size_hints.height = 400;
    size_hints.width = 800;
  }
  else if(options.mode==mandel)
  {
    size_hints.width = 600;
    size_hints.height = 600;
  }
  else
  {
    size_hints.height = 400;
    size_hints.width = 400;
  }

  size_hints.flags = PPosition | PSize;
  
  if (options.geomstring!=NULL) 
  {
    int result;
    result = XParseGeometry(options.geomstring,&size_hints.x,
			    &size_hints.y,(unsigned int*)&size_hints.width,
			    (unsigned int*)&size_hints.height);
    if (result & XNegative)
      size_hints.x += DisplayWidth(display,screen)
	- size_hints.width - options.bwidth*2;
    if (result & YNegative)
      size_hints.y += DisplayHeight(display,screen)
	- size_hints.height - options.bwidth*2;
    if (result & XValue || result & YValue) 
    {
      size_hints.flags |= USPosition;
      size_hints.flags &= ~PPosition;
    }
    if (result & WidthValue || result & HeightValue) 
    {
      size_hints.flags |= USSize;
      size_hints.flags &= ~PSize;
    }
  }

  xswat.override_redirect = False;
  xswat.do_not_propagate_mask = KeyPressMask | KeyReleaseMask ;
  xswat.background_pixmap=None;
/*      |	ButtonPressMask | ButtonReleaseMask; */
/*      vmask = CWOverrideRedirect | CWDontPropagate; */
  vmask =  CWBackPixmap;
  vis_info=(XVisualInfo *)malloc(sizeof(XVisualInfo));
  vis_info->colormap_size=512;
#if defined (__cplusplus) || defined(c_plusplus)
  vis_info->c_class=PseudoColor;  
#else
  vis_info->class=PseudoColor;  
#endif

  vis_info = XGetVisualInfo(display, VisualClassMask|VisualColormapSizeMask,
			    vis_info,&number);
  if(number!=0)
  {
    vis=vis_info->visual;
  }      
  fprintf(stderr,"Vis->map_entries= %d\n",vis->map_entries);
  options.tryfor=vis->map_entries;
  
  for(i=0;i<options.windows;i++)
  {  
    window[i] = XCreateWindow(display, RootWindow(display, screen), 0, 0,
			      size_hints.width, size_hints.height,
			      options.bwidth, CopyFromParent, InputOutput,
			      vis , vmask, &xswat);
    XSetStandardProperties(display,window[i],"Xtacy",basename,
			   None,o_argv,o_argc,&size_hints);
    XSelectInput(display,window[i],
		 StructureNotifyMask|VisibilityChangeMask|
		 ButtonPressMask|ButtonMotionMask);
    XMapWindow(display,window[i]);
    XRaiseWindow(display,window[i]);
  }
  
  free(vis_info);
  return;
}

/* for virtual root windows, split 'em up into sub-windows */
void
makeSubRootWins(int o_argc, char** o_argv)
{
  Window wdontcare;
  int dontcare;
  unsigned int udontcare;
  char *basename;
  unsigned long   vmask;
  XSetWindowAttributes    xswat;
  XSizeHints size_hints;
  int foo,bar,i,x,y;
  unsigned int sCY,sCX;
      
  window=(Window *)malloc(1*sizeof(Window));
  window[0]=RootWindow(display,screen);  
  XGetGeometry(display,window[0],&wdontcare,
	       &dontcare,&dontcare, &sCX, &sCY, &udontcare, &udontcare);
  foo=DisplayWidth(display,screen);
  bar=DisplayHeight(display,screen);
      
/*
  fprintf(stderr,"SuperRoot CX=%d\t SuperRoot CY=%d\n",sCX,sCY);
  fprintf(stderr,"Max visible (%dx%d)... so we can make %d windows\n",
  foo=DisplayWidth(display,screen),
  bar=DisplayHeight(display,screen),
  (sCX/foo)*(sCY/bar));
  */
  if(options.mode == rot_shape || options.mode == cube ||
     options.mode == tet       || options.mode == oct)
  {
    nwindows=1;
    foo=sCX;
    bar=sCY;
  }
  else
  {
    nwindows=((sCX/foo)*(sCY/bar));
  }
  
  /*      printf("SuperRootWin:%lx\n",window); */
	  
  free(window);			/* in every box of Corn Flakes */
  window=(Window *)malloc(sizeof(Window)*nwindows);
  CX=(unsigned int *)calloc(nwindows,sizeof(unsigned int));
  CY=(unsigned int *)calloc(nwindows,sizeof(unsigned int));
  options.windows=nwindows;
  i=0;
  for(x=0;x<(sCX/foo);x++)
    for(y=0;y<(sCY/bar);y++)
    {
      if ((basename=strrchr(progname,'/'))!=NULL) 
	basename++;
      else 
	basename=progname;
      size_hints.flags = PPosition | PSize;
      size_hints.x=DisplayWidth(display,screen);
      size_hints.y=DisplayHeight(display,screen);
		
      xswat.override_redirect = True; 
      vmask = CWOverrideRedirect;
/*
  fprintf(stderr,"Making a new window at %dx%d. (%d,%d)\t",x*foo,
  y*bar, x*foo+foo, y*bar+bar);
  */
      window[i] = XCreateWindow(display, RootWindow(display, screen),
				x*foo, y*bar, foo, bar, 0,
				CopyFromParent, CopyFromParent, vis,
				vmask, &xswat);
      /*
	fprintf(stderr,"Made #%d\n",i);
	*/    
      XSetStandardProperties(display,window[i],"Xtacy",basename,
			     None,o_argv,o_argc,&size_hints);
      XSelectInput(display,window[i],0);
      XLowerWindow(display,window[i]); /* shove the new window
					  down to the bottom */
		
      /*      window=RootWindow(display,screen);  */
      XSync(display,0);
      XMapWindow(display,window[i]);
      XGetGeometry(display,window[i],&wdontcare, &dontcare,&dontcare,
		   &CX[i], &CY[i], &udontcare, &udontcare);
      /*
	fprintf(stderr,"Root CX=%d\t Root CY=%d\n",CX[i],CY[i]);
	printf("RootWin:%lx\n",window[i]);
	*/
      i++;
    }
}

void
makeLockWin(int o_argc,char** o_argv)
{
  unsigned int sCX=0,sCY=0,udontcare;
  int bCX=0,bCY=0,dontcare;
  Window wdontcare;
  char *basename;
  XSetWindowAttributes    xswat;
  int vmask = CWOverrideRedirect|CWEventMask;
  XSizeHints size_hints;

  size_hints.flags = PPosition | PSize;
  size_hints.x=DisplayWidth(display,screen);
  size_hints.y=DisplayHeight(display,screen);
  xswat.override_redirect = True; 
  xswat.event_mask = KeyPressMask | ButtonPressMask;
  if ((basename=strrchr(progname,'/'))!=NULL) 
    basename++;
  else 
    basename=progname;
  window=(Window *)malloc(sizeof(Window)*1);
  XParseGeometry(options.geomstring,&bCX, &bCY,&sCX, &sCY);
  window[0] = XCreateWindow(display, RootWindow(display, screen),
			    bCX, bCY, sCX , sCY, 0,
			    CopyFromParent, CopyFromParent, vis,
			    vmask, &xswat);
  XSetStandardProperties(display,window[0],"Xtacy",basename,
			 None,o_argv,o_argc,&size_hints);
  XSelectInput(display,window[0],
	       ButtonPressMask|KeyPressMask| VisibilityChangeMask);
  XSync(display,0);
  XMapWindow(display,window[0]);
  XFlush(display);
  XRaiseWindow(display,window[0]); /* shove the new window up to the top */

  CX=(unsigned int *)calloc(1,sizeof(unsigned int));
  CY=(unsigned int *)calloc(1,sizeof(unsigned int));
  XGetGeometry(display,window[0],&wdontcare, &dontcare,&dontcare,
	       &CX[0], &CY[0], &udontcare, &udontcare);
}

void
refreshrootquit(int foo)
{ 
/* no clear. leave the current pattern on the background */  
  if(!options.noclear)
  {
    int i;
/*      fprintf(stderr,"Refreshing root and Quiting\n"); */
    s_win_att.backing_store=WhenMapped;
    for(i=0;i<options.windows;i++)
      XChangeWindowAttributes(display,window[i],CWBackingStore,&s_win_att);
      
    XClearWindow(display,RootWindow(display,screen));
    XFreeColors(display, colmap,(unsigned long*)  colors[0],numcolors,1);
    XFreeColors(display, colmap,(unsigned long*)  colors[1],numcolors,1);
    XFreeColors(display, colmap,(unsigned long*)  colors[2],numcolors,1);
    XFlush(display);
  }
  exit(0);
}

void
handle_event(XEvent *event)
{
  int i;
  static int pressed=0;
  static long button_pushed = 0;
  
  do 
  {
    if(event->type<2)
      printf("Funky Event Type: %d\n",event->type);
    else if (event->type==ConfigureNotify)
    {
      if (!startup)
      {
	for(i=0;i<options.windows;i++)
	{
	  if (event->xconfigure.window==window[i]) 
	  {
	    if (CX[i] != event->xconfigure.width ||
		CY[i] != event->xconfigure.height )
	    {
	      XClearWindow(display,event->xconfigure.window);
	      CX[i] = event->xconfigure.width; 
	      CY[i] = event->xconfigure.height;
/* 	    if ((options.mode==kaleid)&&(!options.doroot)) */
/* 	    { */
/* 	      CX[i]>>=1; */
/* 	      CY[i]>>=1; */
	      
/* 	    } */
	      M[i] = max(CY[i],CX[i]);
	      M[i] = M[i] ? M[i] : 1;
	      jj[i]=0;
	      if(options.mode==kaleid)
		randomize_kal(i);
	      /*
		fprintf(stderr,"CX[i]=%d CY[i]=%d M[i]=%d\n",CX[i],CY[i],M[i]);
	      */
	      
	    }
/*		  else
		  jj[i]=1;
*/
	  }
	}
      }
    }
    else if (event->type==MapNotify) 
    {
      if(options.mode==kaleid)
	for(i=0;i<options.windows;i++)
	{
	  if (event->xmap.window==window[i]) 
	  { 
	    randomize_kal(i);
	    
	    HC[i] = rndm((long)numcolors);
	  }
	}
    }
    else if (event->type==VisibilityNotify) 
    {
      for(i=0;i<options.windows;i++)
      {
	if (event->xvisibility.window==window[i]) 
	{
	  if (visible[i] &&
	      event->xvisibility.state == VisibilityFullyObscured) 
	  {
	    visible[i]=0;
	    nvisible--;
	  }
	  else if (!visible[i] &&
		   event->xvisibility.state != VisibilityFullyObscured) 
	  {
	    visible[i]=1;
	    nvisible++;
	  }
	  if ((options.lock) && (!options.debug)) {
	    XRaiseWindow (display, window[0]);	/* shove the window up to the top */		    
	  }
	}
      }
    }
    else if (event->type==ButtonRelease) 
    {
      if((button_pushed & event->xbutton.button) == event->xbutton.button)
      {
	button_pushed ^= event->xbutton.button;	
	if (event->xbutton.button==Button1)
	{ 
/*
 * Zoom in on a section of the Mandelbrot set.. only goes 1 layer of zoom in
 * ok, so I'm lazy and didn't bother figure how to store the new coords
 * deal
 */
	  if(options.mode==mandel)
	  {
	    r=event->xbutton.x;
	    b=event->xbutton.y;
	    printf("Button 1 Released at (%d,%d)\n",r,b);
	    pressed=0;
	    jj[0]=0;
	    dropachicken=1;
	  }
	}
	else if (event->xbutton.button==Button2)
	{ 
	  int i,found=0;
	  for(i=0;i<options.windows;i++)
	  {
	    if(event->xbutton.window==window[i])
	    {
	      found=1; break;
	    }
	  }
	  if(found)
	    jj[i]=0;
	  /* reset screen*/
	  XClearWindow(display,event->xbutton.window);
	  dropachicken=1;
	}
      }
      else 
      {
	
      }
      /* printf("button pushed = %lx\n",button_pushed); */
    }
    else if (event->type==ButtonPress)
    { 
      if((button_pushed & event->xbutton.button) == event->xbutton.button)
      {

      }
      else 
      {
	button_pushed ^= event->xbutton.button; /* flip it on */
	
	if (event->xbutton.button==Button1)
	{ 
/*
 * Zoom in on a section of the Mandelbrot set.. only goes 1 layer of zoom in
 * ok, so I'm lazy and didn't bother figure how to store the new coords
 * deal
 */
	  if(options.mode==mandel)
	  {
	    l=event->xbutton.x;
	    t=event->xbutton.y;
	    printf("Button 1 Pressed at (%d,%d) \n",l,t);
	  }
/* drop a new particle in the chamber */
	  else if(options.mode==gravity)
	  {
	    int i,found=0,winno=0;
	    for(i=0;i<options.windows;i++)
	    {
	      if(event->xbutton.window==window[i])
	      {
		found=1; winno=i; break;
	      }
	    }
	    if(found)
	    {
	      set_part(winno,event->xbutton.x,event->xbutton.y,
		       &partlst[winno][rndm(options.numparts)]);
	    }
	  }
	  else if(options.mode==wandering)
	  {
	    int i,found=0,winno=0;
	    for(i=0;i<options.windows;i++)
	    {
	      if(event->xbutton.window==window[i])
	      {
		found=1; winno=i; break;
	      }
	    }
	    if(found)
	    {
	      x=event->xbutton.x;
	      y=event->xbutton.y;
	    }
	  }
	  else if(options.mode==blob)
	  {
	    int i,found=0,winno=0;
	    for(i=0;i<options.windows;i++)
	    {
	      if(event->xbutton.window==window[i])
	      {
		found=1; winno=i; break;
	      }
	    }
	    if(found)
	    {
	      blobAddFood(winno,event->xbutton.x,event->xbutton.y);
	    }
	  }
	  else if(options.mode==swarm)
	  {
	    int i,found=0,winno=0;
	    for(i=0;i<options.windows;i++)
	    {
	      if(event->xbutton.window==window[i])
	      {
		found=1; winno=i; break;
	      }
	    }
	    if(found)
	    {
	      moveQueen(winno,event->xbutton.x,event->xbutton.y);
	    }
	  }
	  else if(options.mode==tag)
	  {
	    int i,found=0,winno=0;
	    for(i=0;i<options.windows;i++)
	    {
	      if(event->xbutton.window==window[i])
	      {
		found=1; winno=i; break;
	      }
	    }
	    if(found)
	    {
	      moveIt(winno,event->xbutton.x,event->xbutton.y);
	    }
	  }
	  else if(options.mode==life)
	  {
	    int i,found=0,winno=0;
	    for(i=0;i<options.windows;i++)
	    {
	      if(event->xbutton.window==window[i])
	      {
		found=1; winno=i; break;
	      }
	    }
	    if(found)
	    {
	      dropACell(winno,event->xbutton.x,event->xbutton.y);
	    }
	  }
	  else if(!options.mono)
	  {
	    if(options.dynamic_colors)
	      randomize_colors();
	    else
	      rotate_colors();
	  }
	}
	else if (event->xbutton.button==Button3)
	{
	  int i,found=0;
	  /*
	    if(options.mode==tunnel)
	    {
	    EndofTunnel();  is there a light? 
	    }
	  */ 
	  if(options.lock)
	  {
	    maybe_unlock(event);
	  }
	  else
	  {
	    /* do the zoomy close window thing */
	    for(i=0;i<options.windows;i++)
	    {
	      if(event->xbutton.window==window[i])
	      {
		found=1; break;
	      }
	    }
	    if(found)
	    {
	      /* if(nvisible == 1 )
		 fade_to_black();
	      */
	      if(options.annoy_tefler)
	      {
		int x,y,dx,dy;
		unsigned int udontcare;
		Window wdontcare;
	      
		XGetGeometry(display,window[i],&wdontcare,
			     &dx,&dy,(unsigned int*) &x,(unsigned int*)&y,
			     &udontcare, &udontcare);
	  
		while(x>0&&y>0)
		{
		  XMoveResizeWindow(display,window[i],dx+=2,dy+=2,x,y);
		  x=x-4;
		  y=y-4;
		  XFlush(display);
		}
	      }
		  
	      nwindows--;
	      /* options.windows--; */
	      visible[i]=0;
	      nvisible--;
	      /* fprintf(stderr,"Killing Window %d: %lx\n",i,window[i]); */
	      XUnmapWindow(display,window[i]);
	      jj[i]=1; /* don't redraw it */
		  
	      XSync(display,False); 
	      if(nwindows==0)
	      {
		XFreeColors(display, colmap,(unsigned long*) colors[0],
			    numcolors, 1);
		XFreeColors(display, colmap,(unsigned long*) colors[1],
			    numcolors, 1);
		XFreeColors(display, colmap,(unsigned long*) colors[2],
			    numcolors, 1);
		exit(0);
	      }
	    }
	  }
	}
      }
/*      printf("button pushed = %lx\n",button_pushed); */
    }
    
    else if (event->type == KeyPress)
    {
      if (options.lock)
      {
	XPutBackEvent(display, event);
	maybe_unlock (event);
      }
    }
  } while (XCheckMaskEvent(display, ~0L,event)==True);
/*  *event=(XEvent*)0; */
}

void
setup_windows()
{
  int i;
  
  XGetWindowAttributes(display,window[0],&win_att);

  options.bdcolor = StrColor(options.border,WhitePixel(display,screen));
  options.bgcolor = StrColor(options.background,
			     BlackPixel(display,screen));
  
  for(i=0;i<options.windows;i++)
  {
    if(options.perfect)
    {
      colmap = XCreateColormap(display,window[i],vis,AllocNone);
      s_win_att.colormap=colmap;
      XChangeWindowAttributes(display,window[i],CWColormap,&s_win_att);
    }
    else
      colmap= win_att.colormap; 

    XSetWindowBackground(display,window[i],options.bgcolor);
    XSetWindowBorder(display,window[i],options.bdcolor);
    XClearWindow(display,window[i]);

    if (options.totalrand || (options.mode==clover|| options.mode==wandering ||
			      options.mode==plasma|| options.mode==spiral    ||
			      options.mode==mandel|| options.mode==julia     ||
			      options.mode==tunnel|| options.mode==gravity   ||
			      options.mode==newton|| options.mode==test      ||
			      options.mode==dogplasma || options.mode==funky ||
			      options.mode==fields|| options.mode==xstatic  ||options.mode==mixer ||
			      options.mode==waves || options.mode==radial   ||options.mode==ripple ||
			      options.mode==cells || options.mode==taffy    ||options.mode==off   ||
			      options.mode==dline ||options.mode==dcurve    ||options.mode==sunrise||
			      options.mode==blur  || options.mode==flush))
    {
      s_win_att.backing_store=Always;
      XChangeWindowAttributes(display,window[i],CWBackingStore,&s_win_att);
    }
      
    if(options.lock)
    {
      s_win_att.override_redirect=True;
      XChangeWindowAttributes(display,window[i],CWOverrideRedirect,
			      &s_win_att);
    }
      
    intervals[i].tv_sec = (int)(options.delayvalue/1000);
    intervals[i].tv_usec =(int)(options.delayvalue * 1000)%1000000;
    visible[i]=1;
    nvisible++;
  }
}
