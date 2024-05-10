#include <X11/X.h>
#include <X11/Xlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lmath.h"
#include "trippy.h"

/*
   Data on various polyhedra is available at
    http://netlib.att.com/netlib/polyhedra/index.html
    For the icosahedron, the twelve vertices are:

      (+- 1, 0, +-t), (0, +-t, +-1), and (+-t, +-1, 0)

    where t = (sqrt(5)-1)/2, the golden ratio.
*/

typedef struct vt {  /* Structure for individual vertices */
  long lx,ly,lz;    /* Local coordinates of vertex*/
  long wx,wy,wz;    /* World coordinates of vertex */
  long sx,sy,sz;    /* Screen coordinates of vertex*/
} vertex_type;

typedef struct _sha 
{
  vertex_type* vertices;
  vertex_type** faces;
  int maxVis, numVert, numSides, numEdges;
  void (*draw_func)(int,vertex_type**,int);
} SHAPE;

static long matrix[16];  /* Master transformation matrix */
static void matmult(long res[16],long mat1[16],long mat2[16]);
#define matcopy(x,y)   memcpy(x,y,sizeof(long)*16)
static void init_matrix();
static void scale(long);
static void rotate(long,long,long);

/* Transformation functions: */

void
init_matrix()
{
/*  Set the master matrix to |  1  0  0  0 | 
 *                           |  0  1  0  0 |
 *                           |  0  0  1  0 |
 *                           |  0  0  0  1 |
 */

  matrix[1]=matrix[2]=matrix[3]=matrix[4]=
    matrix[6]=matrix[7]=matrix[8]=matrix[9]=
      matrix[11]=matrix[12]=matrix[13]=matrix[14] = 0;
  matrix[0]=matrix[5]=matrix[10]=matrix[15]=1<<16;
}

void
scale(long sf)
{
  /*
   *  Multiply the master matrix by the scaling factor (sf)
   *
   *
   *  Note: assuming that this funciton will be run right after
   *  the init function, we could reduce the code to:
   *     matrix[0]=matrix[5]=matrix[10]=sf;
   *
   * since that is the only change.
   */
  long mat[16];
  long smat[16];

  memset(smat,0,16*sizeof(long));
  smat[0]=smat[5]=smat[10]=sf;
  smat[15]=1<<16;
  
  matmult(mat,smat,matrix);
  matcopy(matrix,mat);
}

void
rotate(long ax, long ay, long az)
{
  long mx[16];
  long mat[16];
  
  long cosx= mCos(ax);
  long sinx= mSin(ax);
  long cosy= mCos(ay);
  long siny= mSin(ay);
  long cosz= mCos(az);
  long sinz= mSin(az);

/*
 * This is 3 matrix multiplies worked out into one.
 * fun fun math (grin)
 */
  mx[0]= (cosy*cosz)>>16;
  mx[1]= (cosy*sinz)>>16;
  mx[2]= -siny>>8;
  mx[4]= (((sinx*siny*cosz)>>24)-((cosx*sinz)>>16));
  mx[5]= (((sinx*siny*sinz)>>24)+((cosx*cosz)>>16));
  mx[6]= (sinx*cosy)>>16;
  mx[8]= (((cosx*siny*cosz)>>24)+((sinx*sinz)>>16));
  mx[9]= (((cosx*siny*sinz)>>24)-((sinx*cosz)>>16));
  mx[10]= (cosx*cosy)>>16;
  mx[3]= mx[7]= mx[11]= mx[12]= mx[13]= mx[14]=0;
  mx[15]=1<<16;
  
  matmult(mat,matrix,mx);
  matcopy(matrix,mat);  
}

void
translate(long xt,long yt,long zt)
{
/*
 *  here we move the shape into position
 */
  long tmat[16];
  long mat[16];

  memset(tmat,0,16*sizeof(long));
  tmat[0]=tmat[5]=tmat[10]=tmat[15]=1<<16;
  tmat[12]=xt<<16; tmat[13]=yt<<16; tmat[14]=zt<<16; 

  matmult(mat,matrix,tmat);
  matcopy(matrix,mat);
}

void
transform(int numvert, vertex_type *vertices)
{
/*
 * and now we run our shape's coordinates through the matrix
 */
  int v;
  vertex_type *vptr=vertices;
  for (v=0; v<numvert; v++)
  {
    vptr->wx=(vptr->lx*matrix[0] + vptr->ly*matrix[4] +
		   vptr->lz*matrix[8] + matrix[12]);
    vptr->wy=(vptr->lx*matrix[1] + vptr->ly*matrix[5] +
		   vptr->lz*matrix[9] + matrix[13]);
    vptr->wz=(vptr->lx*matrix[2] + vptr->ly*matrix[6] +
		   vptr->lz*matrix[10]+ matrix[14]);
    vptr++;
  }
}

void
project(long distance, int numvert, vertex_type* vertices)
{
/* Project shape onto screen */
  int v;
  vertex_type *vptr=vertices;
  
  /* Loop though vertices: */
  for (v=0; v<numvert; v++,vptr++)
  {
    long fx,fy;
    
    /* Divide world x & y coords by z coords: */
/* if vptr->wz = 0,  something is not right with the world */
    fx= (distance/vptr->wz);
    vptr->sx = fx*vptr->wx;
    fy= (distance/vptr->wz);
    vptr->sy = fy*vptr->wy;
  }
}

void
matmult(long result[16],long mat1[16], long mat2[16])
{
  int i,j,k;
    
/* Multiply matrix MAT1 by matrix MAT2,
 *  returning the result in RESULT
 */
  long *res = result;
  for (i=0; i<4; i++)
  {
    int i4 = 4*i;
    for (j=0; j<4; j++)
    {
      *res  = mat1[i4]   * mat2[j];
      *res += mat1[i4+1] * mat2[4+j];
      *res += mat1[i4+2] * mat2[8+j];
      *res += mat1[i4+3] * mat2[12+j];
      res++;
    }
  }
  return;
}

void
drawTriFace(int winno, vertex_type **vertices,int clr)
{
  int i;
  XPoint pts[3];
  long x1= vertices[0]->sx;
  long y1= vertices[0]->sy;
  long x2= vertices[1]->sx;
  long y2= vertices[1]->sy;
  long x3= vertices[2]->sx;
  long y3= vertices[2]->sy;

  long dx1 = (x3-x1) /options.number;
  long dy1 = (y3-y1) /options.number; 
  long dx2 = (x3-x2) /options.number; 
  long dy2 = (y3-y2) /options.number; 
  long XORIGIN= CX[winno]<<15;
  long YORIGIN= CY[winno]<<15;

/* center the shape on the window */  
  x1+=XORIGIN; x2+=XORIGIN; x3+=XORIGIN;
  y1+=YORIGIN; y2+=YORIGIN; y3+=YORIGIN;

/*
  pts[0].x=x1; pts[0].y=y1;
  pts[1].x=x2; pts[1].y=y2;
  pts[2].x=x3; pts[2].y=y3;
  XFillPolygon(display,window[winno],color_gcs[clr],pts,3,
               Convex,CoordModeOrigin);
*/
  XDrawLine(display,window[winno],color_gcs[clr],x1>>16,y1>>16,x3>>16,y3>>16);
  XDrawLine(display,window[winno],color_gcs[clr],x2>>16,y2>>16,x3>>16,y3>>16);

  for(i=0;i<=options.number;i++)
  {
    XDrawLine(display,window[winno],color_gcs[clr],x1>>16,y1>>16,x2>>16,y2>>16);
    x1+=dx1; y1+=dy1;
    x2+=dx2; y2+=dy2;
  }
}

void
drawSquareFace(int winno,vertex_type **vertices,int clr)
{
  int i;
  long x1= vertices[0]->sx;
  long y1= vertices[0]->sy;
  long x2= vertices[3]->sx;
  long y2= vertices[3]->sy;
  long x3,x4,y3,y4;
  long dx1 = (((x3=vertices[1]->sx)-x1)) /options.number;
  long dy1 = (((y3=vertices[1]->sy)-y1)) /options.number; 
  long dx2 = (((x4=vertices[2]->sx)-x2)) /options.number; 
  long dy2 = (((y4=vertices[2]->sy)-y2)) /options.number; 
  long XORIGIN= CX[winno]<<15;
  long YORIGIN= CY[winno]<<15;
  
  x1+=XORIGIN; x2+=XORIGIN; x3+=XORIGIN; x4+=XORIGIN;
  y1+=YORIGIN; y2+=YORIGIN; y3+=YORIGIN; y4+=YORIGIN;

  XDrawLine(display,window[winno],color_gcs[clr],x1>>16,y1>>16,x3>>16,y3>>16);
  XDrawLine(display,window[winno],color_gcs[clr],x2>>16,y2>>16,x4>>16,y4>>16);

  fprintf(stderr, "(%ld,%ld),(%ld,%ld),(%ld,%ld),(%ld,%ld)\n",x1>>16,y1>>16,
  	  x2>>16,y2>>16,x3>>16,y3>>16,x4>>16,y4>>16);
  for(i=0;i<=options.number;i++)
  {
    XDrawLine(display,window[winno],color_gcs[clr],x1>>16,y1>>16,x2>>16,y2>>16);
    x1+=dx1; y1+=dy1;
    x2+=dx2; y2+=dy2;
  }
}

#if 0
int
backface(vertex_type **vertices)
{
/* 	 Returns 0 if POLYGON is visible, -1 if not.
 *   POLYGON must be part of a convex polyhedron
 */
   int z; 
   vertex_type *v0,*v1,*v2;  /* Pointers to three vertices */
   /* only need 3 vertices .. 3 points make a plane and all that */
   v0=vertices[0];
   v1=vertices[1];
   v2=vertices[2];
   z= (v1->sx-v0->sx) * (v2->sy-v0->sy) - (v1->sy-v0->sy) * (v2->sx-v0->sx);
   return(z>=0);
}
#endif
#define backface(verts) (((verts[1]->sx-verts[0]->sx)*\
			  (verts[2]->sy-verts[0]->sy)-\
			  (verts[1]->sy-verts[0]->sy)*\
			  (verts[2]->sx-verts[0]->sx))>=0)

void
draw_shape(int winno, int clr, SHAPE *shape)
{
  int i;
  int j=0;
  vertex_type ** faces;
  int edges;

  edges = shape->numEdges;
  faces = shape->faces;
  for(i=0; i<shape->numSides; i++)
  {
    if (backface(faces))
    {
      shape->draw_func(winno,faces,
       (clr==1)?0:(numcolors*(i%5)/6)+5); 
      j++;
    }
    if(j>shape->maxVis)
      break;
    faces += edges;
  } 
}

/* and now, define the shapes */

vertex_type oct_vertices[]=
{
 {20<<16, 0, 0,
  0,  0, 0,
  0,  0, 0},
 {-20<<16, 0, 0,
  0,  0, 0,
  0,  0, 0},
  {0, 20<<16, 0,
  0,  0, 0,
  0,  0, 0},
  {0,-20<<16,0,
  0,  0, 0,
  0,  0, 0},
 {0,  0,20<<16,
  0,  0, 0,
  0,  0, 0},
 {0,  0,-20<<16,
  0,  0,  0,
  0,  0,  0},
};

vertex_type *oct_faces[]=
{
  &oct_vertices[0],&oct_vertices[4],&oct_vertices[2],
  &oct_vertices[1],&oct_vertices[2],&oct_vertices[4],
  &oct_vertices[0],&oct_vertices[3],&oct_vertices[4],
  &oct_vertices[1],&oct_vertices[4],&oct_vertices[3],
  &oct_vertices[0],&oct_vertices[2],&oct_vertices[5],
  &oct_vertices[1],&oct_vertices[5],&oct_vertices[2],
  &oct_vertices[0],&oct_vertices[5],&oct_vertices[3],
  &oct_vertices[1],&oct_vertices[3],&oct_vertices[5]
};

SHAPE octo=
{
  oct_vertices,oct_faces,4,6,8,3,drawTriFace
};


vertex_type tet_vertices[]=
{
 {10<<16, 10<<16, 10<<16,
  0,  0, 0,
  0,  0, 0},
  {10<<16,-10<<16,-10<<16,
  0,  0, 0,
  0,  0, 0},
 {-10<<16,10<<16,-10<<16,
  0,  0, 0,
  0,  0, 0},
 {-10<<16,-10<<16,10<<16,
    0,  0,0,
    0,  0,0},
};

vertex_type *tet_faces[]=
{
  &tet_vertices[0], &tet_vertices[1],&tet_vertices[3],
  &tet_vertices[2], &tet_vertices[1],&tet_vertices[0],
  &tet_vertices[3], &tet_vertices[2],&tet_vertices[0],
  &tet_vertices[1], &tet_vertices[2],&tet_vertices[3]
};

SHAPE tetra=
{
  tet_vertices,tet_faces,3,4,4,3,drawTriFace
};

vertex_type cube_vertices[]=
{  /* Vertices for cube */
  {-10<<16,-10<<16,10<<16,		
     0,  0, 0,
     0,  0, 0},
  {10<<16,-10<<16,10<<16,		
    0,  0, 0,
    0,  0, 0},
  {10<<16,10<<16,10<<16,		
    0, 0, 0,
    0, 0, 0},
  {-10<<16,10<<16,10<<16,		
     0, 0, 0,
     0, 0, 0},
  {-10<<16,-10<<16,-10<<16,		
     0,  0,  0,
     0,  0,  0},
  {10<<16,-10<<16,-10<<16,		
    0,  0,  0,
    0,  0,  0},
  {10<<16,10<<16,-10<<16,	     
    0, 0,  0,
    0, 0,  0},
  {-10<<16,10<<16,-10<<16,
   0,0,0,
   0,0,0
  }
};

vertex_type *cube_faces[24] =
{
  &cube_vertices[0],&cube_vertices[1],&cube_vertices[5],&cube_vertices[4],
  &cube_vertices[5],&cube_vertices[6],&cube_vertices[7],&cube_vertices[4],
  &cube_vertices[2],&cube_vertices[6],&cube_vertices[5],&cube_vertices[1],
  &cube_vertices[6],&cube_vertices[2],&cube_vertices[3],&cube_vertices[7],
  &cube_vertices[2],&cube_vertices[1],&cube_vertices[0],&cube_vertices[3],
  &cube_vertices[4],&cube_vertices[7],&cube_vertices[3],&cube_vertices[0]
};

SHAPE cubik=
{
  cube_vertices,cube_faces,4,8,6,4,drawSquareFace
};

#define NUMSTEPS 124  /* twelve step program to change */

void
morph(int winno, SHAPE* source, SHAPE* dest) 
{
  SHAPE temp;
  int i,step;
  long *dx , *dy, *dz;
  long *newx, *newy, *newz;
  
  fprintf (stderr,"Morphing from %d to %d\n",source->numVert,dest->numVert);
  
  if(source->numVert > dest->numVert)
  {
    temp.numVert=source->numVert;
    temp.numSides=source->numSides;
    temp.numEdges=source->numEdges;
    temp.maxVis=source->maxVis;
  }
  else
  {
    temp.numVert=dest->numVert;
    temp.numSides=dest->numSides;
    temp.numEdges=dest->numEdges;
    temp.maxVis=dest->maxVis;
  }
  
  dx=(long*)malloc(sizeof(long)*temp.numVert);
  dy=(long*)malloc(sizeof(long)*temp.numVert);
  dz=(long*)malloc(sizeof(long)*temp.numVert);
  newx=(long*)calloc(temp.numVert,sizeof(long));
  newy=(long*)calloc(temp.numVert,sizeof(long));
  newz=(long*)calloc(temp.numVert,sizeof(long));
  temp.vertices=(vertex_type*)malloc(sizeof(vertex_type)*temp.numVert);
  temp.faces=(vertex_type**)malloc(sizeof(vertex_type*)*temp.numSides*
				   temp.numEdges);

  if(source->numVert > dest->numVert)
  {
    memcpy(temp.vertices,source->vertices,
	   sizeof(vertex_type)*source->numVert);
    for(i=0;i<(source->numSides*source->numEdges);i++)
    {
/* wheee.. fun with pointer math ... but will it work everywhere? */
/* eh... what the hell, it works under Linux/GCC */
      int base=(int)&(source->vertices[0]);
      int facenum =(int) (source->faces[i]);
      int sidenum= (facenum-base)/(sizeof(vertex_type));
      temp.faces[i]= &temp.vertices[sidenum];
    }
    for(i=0;i<dest->numVert;i++)
    {
      dx[i]=(long)(dest->vertices[i].lx-source->vertices[i].lx)/NUMSTEPS;
      dy[i]=(long)(dest->vertices[i].ly-source->vertices[i].ly)/NUMSTEPS;
      dz[i]=(long)(dest->vertices[i].lz-source->vertices[i].lz)/NUMSTEPS;
      newx[i]=source->vertices[i].lx;
      newy[i]=source->vertices[i].ly;
      newz[i]=source->vertices[i].lz;
    }
    for(;i<temp.numVert;i++)
    {
      dx[i]=(long)(-source->vertices[i].lx)/NUMSTEPS;
      dy[i]=(long)(-source->vertices[i].ly)/NUMSTEPS;
      dz[i]=(long)(-source->vertices[i].lz)/NUMSTEPS;
      newx[i]=source->vertices[i].lx;
      newy[i]=source->vertices[i].ly;
      newz[i]=source->vertices[i].lz;
    }
  }
  else
  {
    memcpy(temp.vertices,dest->vertices,
	   sizeof(vertex_type)*dest->numVert);
    for(i=0;i<(dest->numSides*dest->numEdges);i++)
    {
/* wheee.. fun with pointer math ... but will it work everywhere? */
      int base=(int) &(dest->vertices[0]);
      int facenum =(int) (dest->faces[i]);
      int sidenum = (facenum-base)/sizeof(vertex_type);
/*      fprintf(stderr,"Sidenum %d = %d\t",i,sidenum); */
      temp.faces[i]= &temp.vertices[sidenum];
    }
    for(i=0;i<source->numVert;i++)
    {
      dx[i]=(long)(dest->vertices[i].lx-source->vertices[i].lx)/NUMSTEPS;
      dy[i]=(long)(dest->vertices[i].ly-source->vertices[i].ly)/NUMSTEPS;
      dz[i]=(long)(dest->vertices[i].lz-source->vertices[i].lz)/NUMSTEPS;
      newx[i]=source->vertices[i].lx;
      newy[i]=source->vertices[i].ly;
      newz[i]=source->vertices[i].lz;

    }
    for(;i<temp.numVert;i++)
    {
      dx[i]=(long)(dest->vertices[i].lx)/NUMSTEPS;
      dy[i]=(long)(dest->vertices[i].ly)/NUMSTEPS;
      dz[i]=(long)(dest->vertices[i].lz)/NUMSTEPS;
      newx[i]=0;
      newy[i]=0;
      newz[i]=0;
    }
  }
		      
  if(source->draw_func==dest->draw_func)
  {
    temp.draw_func=source->draw_func;
  }
  else
  {
    temp.draw_func=drawSquareFace; /*this func should deal with everything */
  }
/* ok, now the shape is defined */
  
  for(step=0;step<NUMSTEPS;step++)
  {
    for(i=0;i<temp.numVert;i++)
    {
      newx[i]+= dx[i];
      temp.vertices[i].lx = (int)newx[i];
      newy[i]+= dy[i];
      temp.vertices[i].ly = (int)newy[i];
      newz[i]+= dz[i];
      temp.vertices[i].lz = (int)newz[i];
    }
  
    transform(temp.numVert, temp.vertices);
    draw_shape(winno,1,&temp);			/* clear */
    project(400,temp.numVert,temp.vertices);	/* calculate */
    draw_shape(winno,0,&temp);            	/* and draw */ 
  }


 /* all done, clean up */
  draw_shape(winno,1,&temp);			/* clear */
  free(newx);
  free(newy);
  free(newz);
  free(dx);
  free(dy);
  free(dz);
  free(temp.faces);
  free(temp.vertices);
  return;
}

void
exit_maxh()
{
  return;
}

void
exit_cube()
{
  return;
}

void
exit_oct()
{
  return;
}

void
exit_tet()
{
  return;
}

void
exit_shape()
{
  return;
}

void
rotate_shape(int winno)
{
  static long xangle = 0, yangle= 0, zangle = 0;
  static long xrot = 10, yrot=10, zrot=10;
  static int xloc=0, yloc=0, zloc=50;
  static int dist=400, distrot=0;
  static char init=0;
  
  static SHAPE* shape;

  if(!init) 
  {
    switch(options.mode)
    {
      case tet:
	shape=&tetra; break;
      case oct: 
        shape=&octo;  break;
      case cube:
        shape=&cubik;  break;
      case rot_shape:
      default:
	shape=&cubik;  break;
    }
    
    init=1;
  }
  
  winno=0;  /* only deal with one window... for now */
  
  init_matrix();                    
  scale(70);
  rotate(xangle,yangle,zangle);   
  xangle += xrot;                   
  yangle += yrot;                    
  zangle += zrot;
  dist += distrot;
  translate(xloc,yloc,50); 
  
  transform(shape->numVert, shape->vertices);
                                    
  draw_shape(winno,1,shape);			/* clear */

  project((options.mode==maxh)?(max(CX[winno],CY[winno])*2):dist,shape->numVert,
		shape->vertices);	/* calculate */
  
  draw_shape(winno,0,shape);            	/* and draw */ 

  if(rndm(500)==0) xrot*=-1;   
  if(rndm(500)==0) yrot*=-1;  
  if(rndm(500)==0) zrot*=-1;
  /*  if(rndm(500)==0) distrot+=(rndm(3)-1); */
  if(rndm(100)==0 && options.mode==rot_shape)
  {
    if(shape== &octo)
    {
	draw_shape(winno,1,shape);			/* clear */
	shape=(rndm(10)>4?&cubik:&tetra);
	morph(winno,&octo,shape);
    }
    else if (shape== &cubik)
    {
      draw_shape(winno,1,shape);			/* clear */
      shape=(rndm(10)>4?&octo:&tetra);
      morph(winno,&cubik,shape);
    }
    else if (shape == &tetra)
    {
      draw_shape(winno,1,shape);			/* clear */
      shape=(rndm(10)>4?&octo:&cubik);
      morph(winno,&tetra,shape);
    }
    else
    {
      fprintf(stderr,"Whoa! Bizarre shape.\n");
      exit(0);
    }
  }
  return;
}
