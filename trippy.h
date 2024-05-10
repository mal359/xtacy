/*
 *  Include file for Xtacy
 *
 */

struct particle
{
  double x,y;
  double vx,vy;
  int mass;
  int color;
};


typedef enum {kaleid,boxes,bozo,test,circ,wandering,qix,qix4,clover,tunnel,plasma,rose,spiral,gravity,off,spheres,funky,ripple,life,xstatic,maxh,fields,starback,mixer,cells,taffy,bez,swarm,dogplasma,blob,rot_shape,cube,tet,oct,waves,radial,fracrect,tag,sunrise,dline,dcurve,lunch,flush,blur,lightning,newton,mandel,julia,munch,sunflower} modes;

#define MIR_VERT 0x1
#define MIR_HORIZ 0x2

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif  

XPoint *wrecked(int,int,int,int);
XPoint *make_tri (int,int,int,int,int,int);
XPoint *make_circle(int,int,int,int);
XPoint *Bezier(XPoint*, int);

unsigned long StrColor(char *,unsigned long);
void parse_options(int,char**);
void bounce(int);
void draw_lines(int);
void draw_bozo(int);
void draw_boxes(int);
void draw_qix(int);
void draw_circ(int);
void draw_kaleid(int);
void draw_test(int);
void draw_off(int);
void draw_static(int);
void draw_sphere(int);
void draw_clover(int);
void draw_rose(int);
void draw_plasma(int);
void twiddle_plasma(int,int,int,int,int);

void draw_sunrise(int);
void draw_spiral(int);
void draw_max(int);
int draw_gravity(int);
void init_gravity(int);
void set_part(int,int,int,struct particle*);
void init_field(int);
void draw_field(int);
void fill_tris(int);
void fill_rects(int);
void fill_circles(int);
void init_frame(int);
void move_frame(int);
void draw_tunnel(int);
void draw_mandel(int,int,int,int,int);
void draw_julia(int,double,double);
void draw_newton(int);
void draw_ripple(int);
void rotate_colors();
void randomize_colors();
int randomize_color();
void get_them_colors();
void wander(int,int *,int *);
void draw_funky(int);
void randomize_kal(int);
void usage();
/* void EndofTunnel(); */
void initTables();
void makeBlankCursor();
int init_kaleid(int);
void initBlob(int);
void getALife(int);
void init_galaxy(int);
void init_swarm(int);
void init_playground(int);
/* void draw_xload(int); */
void moveBlob(int);
void rotate_shape(int);
void lightning_strike(int);
void de_strike(int);
void drawLife(int);
void draw_stars(int);
void draw_playground(int);
void draw_swarm(int);
void setup_windows();
void draw_fracrect(int,int,int,int,int,int,int);
void draw_taffymaker(int);
void draw_mixer(int);
void draw_cells(int);
void draw_dogplasma(int);
void draw_bez(int);
void draw_qix4(int);
void moveQueen(int,long,long);
void moveIt(int,long,long);
void dropACell(int,int,int);
void init_rose(void);
  XPoint* mirror(int, int,  int, int, int);
  void init_munch(int);
  void init_flush(int);
  void init_blur(int);
  void draw_munch(int,int);
  void draw_flush(int);
  void draw_blur(int);
  void draw_lunch(int,int*);
  void draw_sunflower(int);
  void draw_dline(int);
  void draw_dcurve(int);
  void exit_blur();
  void exit_flush();
  void blobAddFood(int,int,int);
  void make_white(int);

#if 0
void init_oscill(int);
void draw_oscill(int);
#endif
void handle_event(XEvent *);

void exit_blob();
void exit_fields();
void exit_frame(int winno);
void exit_bozo(int winno);
void exit_circ(int winno);
void exit_qix(int winno);
void exit_qix4(int winno);
void exit_bez(int winno);
void exit_gravity();
void exit_kaleid();
void exit_life();
void exit_lightning();
void exit_oscill(int winno);
void exit_lunch()	/* HUWAARRRF! */;
void exit_plasma();
void exit_dogplasma();
void exit_fracrect();
void exit_spiral();
void exit_mandel();
void exit_julia();
void exit_newton();
void exit_wandering();
void exit_funky();
void exit_ripple();
void exit_xstatic();
void exit_mixer();
void exit_cells();
void exit_dcurve();
void exit_dline();
void exit_sunrise();
void exit_taffy();
void exit_tunnel();
void exit_clover();
void exit_test();
void exit_sphere();
void exit_off();
void exit_munch();
void exit_munch();
void exit_rose();
void exit_maxh();
void exit_cube();
void exit_oct();
void exit_tet();
void exit_shape();
void exit_maxh();
void exit_cube();
void exit_oct();
void exit_tet();
void exit_shape();
void exit_starback();
void exit_swarm();
void exit_playground(int winno);
void exit_tag();
void exit_waves();
void exit_radial();
void exit_xload();
#define NCOLORS 258
#ifdef SYSV
#define rndm(x) (rand()%(x?x:1))
#define srndm(x) (srand(x))
#else
#define rndm(x) (random()%(x?x:1))
#define srndm(x) (srandom(x))
#endif
#define max(x,y) ((x>y)?x:y)
#define min(x,y) ((x<y)?x:y)

Display *display;
Screen *scr;

int screen;
int nwindows, nvisible;
int *visible;
Window *window;
Colormap colmap; /* One map to bind them... */
unsigned int *CX, *CY, *M, *HC;

int numcolors;
int share_colors;
GC *color_gcs;
long** colors; /* [NCOLORS][3]; */
double SinTbl[32];
extern Cursor mycursor ;

char *progname;

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

/* opt1 is a catch-all 2nd option number */
#define numparts opt1
#define trell opt1
#define norect opt1

struct foo
{
  unsigned long bgcolor;
  unsigned long bdcolor;
  int palette; /* wheee! different palettes */  
  int bwidth;  /* border width */
  int number;  /* number of whatevers */
  int opt1;    /* secondary option thingy */
  float delayvalue;
  int noclear;
  int annoy_tefler;
  int doroot;
  int mono;
  int multi;
  int dynamic_colors;
  int totalrand;
  int tryfor;
  int perfect;
  int inqPassword; /* inquire password instead of getting it from system */
  int allowroot; /* allowroot password as well (when inqPassword is 0) */
  int debug;
  int lock;  /* Lock the Display */
  int rillybig;
  int windows;
  int mirrors;
  int maxcycles;
  modes mode;
  long win;
  char *displayname;
  char *geomstring;
  char *background;
  char *border;
  char *boxc;
  char *palette_filename;
#ifdef JPEG_SUPPORT
  char *loadfile;
#endif
};

extern struct foo options;

