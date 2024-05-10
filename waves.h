/*
	wave table management.
*/

#define	MAXWAVES	100

/* these are libcalls */
#if defined (__cplusplus) || defined(c_plusplus)
extern "C"  {
#endif
void L_HSineLine();
void L_VSineLine();
void L_HSawLine();
void L_VSawLine();
void L_HWaveLine();
void L_VWaveLine();
void L_WaveNew();
void L_WaveSet();
void L_Radial();

/* functions */
void wave_init();	/* initialise internal waves */

/* allocate a new wave */
long wave_new(long size);
long wave_new_table(long size,unsigned short *tab);

/* set a value in a wave */
void wave_set(long wavindex,long index,short val);

/* draw a wave */
void wave_drawh(int winno,long wavindex,long x,long y,long len,long offset,
			   long scale,long intensity,long paloffset);

void wave_drawv(int winno,long wavindex,long x,long y,long len,long offset,
			   long scale,long intensity,long paloffset);

void wave_radial(int, long ,long ,long , long ,long ,long ,long );

unsigned short* get_wave(int);

#if defined (__cplusplus) || defined(c_plusplus)
}
#endif
