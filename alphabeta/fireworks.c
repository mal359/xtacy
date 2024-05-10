/*
 *  Fireworks.c
 *
 *
 */ 

static int inited=0;

static int*    birth, death, x,y, z,xp, yp,zp,g, pos,
  step, roty, rotnumy, rotframe, scale, scaleinc, mtl;

void
init_fireworks()
{
  if(!inited)
  {
    inited=1;
    birth=(int*)malloc(options.number*sizeof(int));
    death=(int*)malloc(options.number*sizeof(int));
    x=(int*)malloc(options.number*sizeof(int));
    y=(int*)malloc(options.number*sizeof(int));
    z=(int*)malloc(options.number*sizeof(int));
    xp=(int*)malloc(options.number*sizeof(int));
    yp=(int*)malloc(options.number*sizeof(int));
    zp=(int*)malloc(options.number*sizeof(int));
    g=(int*)malloc(options.number*sizeof(int));
    pos=(int*)malloc(options.number*sizeof(int));
    step=(int*)malloc(options.number*sizeof(int));
    roty=(int*)malloc(options.number*sizeof(int));
    rotnumy=(int*)malloc(options.number*sizeof(int));
    rotframe=(int*)malloc(options.number*sizeof(int));
    scale=(int*)malloc(options.number*sizeof(int));
    scaleinc=(int*)malloc(options.number*sizeof(int));
    mtl =(int*)malloc(options.number*sizeof(int));
  }
}

void
draw_fireworks (int winno) 
{
  int numpart=options.number;

  int totframe=100;
  int firststep=(M_PI*0.5)/totframe;
  int gravity_max=9;
  int part;
  
  for(part=1;part<=numpart;part++)
  { 
    birth[part]=0;
    death[part]=totframe;
    x[part]=(0-(rndm(5))+(rndm(5)));
    y[part]=(0-(rndm(5))+(rndm(5)));
    z[part]=(0-(rndm(5))+(rndm(5)));
    
    xp[part]=0;
    yp[part]=0;
    zp[part]=0;
    g[part]=rndm(gravity_max);
    pos[part]=0;
    step[part]=firststep*(rand()+.5);
    roty[part]=0;
    rotnumy[part]=rand()*1.5;
    rotframe[part]=0+(rndm(totframe*0.1));
    scale[part]=1;
    scaleinc[part]=rand(0.1);
    mtl[part]=rndm(5);
  }
  
  for(frame=0;frame<=totframe;frame++)
  {
    for(part=1;part<=numpart;part++)
    {
      if(frame>=birth[part]&&frame<=death[part]) /*is particle alive*/
      {
	xp[part]=xp[part]+x[part];
	yp[part]=(yp[part]+y[part])-(sin(pos[part])*g[part]);
	zp[part]=zp[part]+z[part];
	pos[part]=pos[part]+step[part];

	if(frame>=rotframe[part]) /* should particle rotate to twinkle and
				     start scale to 0 */
	{
	  roty[part]=roty[part]+rotnumy[part];
	  scale[part]=scale[part]-scaleinc[part];
	  if(scale[part]<0) { scale[part]=0 }
	}
	XDrawPoint(display,window[winno],color_gcs[zp[part]%numcolors],
		   xp[part],yp[part]);
      }
    }
  }
}





