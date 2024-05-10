/*
	Math routines and lookup table management
*/

#ifndef M_PI
#define M_PI 3.14159265
#endif

#ifndef M_PI_2
#define M_PI_2 (M_PI / 2)
#endif

extern signed int sinTab[4096];
#if defined (__c_plusplus) || defined (c_plusplus)
extern "C"  {
#endif  
void math_init();
int mSin(int x);		/* sin: x=0-4096, res=-32768-32767 */
int mCos(int x);		/* cos: x=0-4096, res=-32768-32767 */
unsigned int math_dist(int x1,int y1,int x2,int y2);
unsigned int line_dist(int ,int ,int ,int, int, int );
unsigned int curve_dist(int,int,XPoint*,int );
/* distance twixt two points */
XPoint PolartoRect(float,float);
double RecttoPolar(int,int);
#if defined (__c_plusplus) || defined (c_plusplus)
}
#endif


