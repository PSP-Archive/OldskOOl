#define PGA_CHANNELS 3
#define PGA_SAMPLES 256
#define MAXVOLUME 0x8000
#define SND1_MAXSLOT 16
#define PB_N 32
#define WAVFILEMAX_BG 8*1024*1024
#define O_RDONLY    0x0001 
#define O_WRONLY    0x0002 
#define O_RDWR      0x0003 
#define O_NBLOCK    0x0010 
#define O_APPEND    0x0100 
#define O_CREAT     0x0200 
#define O_TRUNC     0x0400 
#define O_NOWAIT    0x8000 
#define MAX_PATH 512
#define NULL 0
//---------- variables
int pga_ready=0;
int pga_handle[PGA_CHANNELS];
short pga_sndbuf[PGA_CHANNELS][2][PGA_SAMPLES][2];
int pga_threadhandle[PGA_CHANNELS];
volatile int pga_terminate=0;

typedef int (*pg_threadfunc_t)(int args, void *argp);

typedef struct {
	unsigned long channels;
	unsigned long samplerate;
	unsigned long samplecount;
	unsigned long datalength;
	char *wavdata;
	unsigned long rateratio;		// samplerate / 44100 * 0x10000
	unsigned long playptr;
	unsigned long playptr_frac;
	int playloop;
} wavout_wavinfo_t;

wavout_wavinfo_t *wavout_snd0_wavinfo=0;
wavout_wavinfo_t wavout_snd1_wavinfo[SND1_MAXSLOT];

int wavout_snd1_playing[SND1_MAXSLOT];


int wavout_snd0_ready=0;
unsigned long wavout_snd0_playptr=0;
int wavout_snd0_playend=0;


short powerbuf[PB_N][256];
unsigned int powerbuf_in=0;

short powersrc[256];
short powersrc2[256];


char wavdata_bg[WAVFILEMAX_BG];
wavout_wavinfo_t wavinfo_bg;
char pg_mypath[MAX_PATH];
char pg_workdir[MAX_PATH];

//------------ Tableau de merde
int brtbl[]={
  0,128, 64,192, 32,160, 96,224, 16,144, 80,208, 48,176,112,240,  8,136, 72,200, 40,168,104,232, 24,152, 88,216, 56,184,120,248,
  4,132, 68,196, 36,164,100,228, 20,148, 84,212, 52,180,116,244, 12,140, 76,204, 44,172,108,236, 28,156, 92,220, 60,188,124,252,
  2,130, 66,194, 34,162, 98,226, 18,146, 82,210, 50,178,114,242, 10,138, 74,202, 42,170,106,234, 26,154, 90,218, 58,186,122,250,
  6,134, 70,198, 38,166,102,230, 22,150, 86,214, 54,182,118,246, 14,142, 78,206, 46,174,110,238, 30,158, 94,222, 62,190,126,254,
  1,129, 65,193, 33,161, 97,225, 17,145, 81,209, 49,177,113,241,  9,137, 73,201, 41,169,105,233, 25,153, 89,217, 57,185,121,249,
  5,133, 69,197, 37,165,101,229, 21,149, 85,213, 53,181,117,245, 13,141, 77,205, 45,173,109,237, 29,157, 93,221, 61,189,125,253,
  3,131, 67,195, 35,163, 99,227, 19,147, 83,211, 51,179,115,243, 11,139, 75,203, 43,171,107,235, 27,155, 91,219, 59,187,123,251,
  7,135, 71,199, 39,167,103,231, 23,151, 87,215, 55,183,119,247, 15,143, 79,207, 47,175,111,239, 31,159, 95,223, 63,191,127,255,
};

int mm1[8][128]={
{  16384,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{  16384,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{  16384, 11585,     0,-11585,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{  16384, 15136, 11585,  6269,     0, -6269,-11585,-15136,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{  16384, 16069, 15136, 13622, 11585,  9102,  6269,  3196,     0, -3196, -6269, -9102,-11585,-13622,-15136,-16069,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{  16384, 16305, 16069, 15678, 15136, 14449, 13622, 12665, 11585, 10393,  9102,  7723,  6269,  4756,  3196,  1605,     0, -1605, -3196, -4756, -6269, -7723, -9102,-10393,-11585,-12665,-13622,-14449,-15136,-15678,-16069,-16305,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{  16384, 16364, 16305, 16206, 16069, 15892, 15678, 15426, 15136, 14810, 14449, 14053, 13622, 13159, 12665, 12139, 11585, 11002, 10393,  9759,  9102,  8423,  7723,  7005,  6269,  5519,  4756,  3980,  3196,  2404,  1605,   803,     0,  -803, -1605, -2404, -3196, -3980, -4756, -5519, -6269, -7005, -7723, -8423, -9102, -9759,-10393,-11002,-11585,-12139,-12665,-13159,-13622,-14053,-14449,-14810,-15136,-15426,-15678,-15892,-16069,-16206,-16305,-16364,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{  16384, 16379, 16364, 16339, 16305, 16260, 16206, 16142, 16069, 15985, 15892, 15790, 15678, 15557, 15426, 15286, 15136, 14978, 14810, 14634, 14449, 14255, 14053, 13842, 13622, 13395, 13159, 12916, 12665, 12406, 12139, 11866, 11585, 11297, 11002, 10701, 10393, 10079,  9759,  9434,  9102,  8765,  8423,  8075,  7723,  7366,  7005,  6639,  6269,  5896,  5519,  5139,  4756,  4369,  3980,  3589,  3196,  2801,  2404,  2005,  1605,  1205,   803,   402,     0,  -402,  -803, -1205, -1605, -2005, -2404, -2801, -3196, -3589, -3980, -4369, -4756, -5139, -5519, -5896, -6269, -6639, -7005, -7366, -7723, -8075, -8423, -8765, -9102, -9434, -9759,-10079,-10393,-10701,-11002,-11297,-11585,-11866,-12139,-12406,-12665,-12916,-13159,-13395,-13622,-13842,-14053,-14255,-14449,-14634,-14810,-14978,-15136,-15286,-15426,-15557,-15678,-15790,-15892,-15985,-16069,-16142,-16206,-16260,-16305,-16339,-16364,-16379, },
};
int mm2[8][128]={
{      0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{      0,-16384,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{      0,-11585,-16384,-11585,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{      0, -6269,-11585,-15136,-16383,-15136,-11585, -6269,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{      0, -3196, -6269, -9102,-11585,-13622,-15136,-16069,-16383,-16069,-15136,-13622,-11585, -9102, -6269, -3196,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{      0, -1605, -3196, -4756, -6269, -7723, -9102,-10393,-11585,-12665,-13622,-14449,-15136,-15678,-16069,-16305,-16384,-16305,-16069,-15678,-15136,-14449,-13622,-12665,-11585,-10393, -9102, -7723, -6269, -4756, -3196, -1605,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{      0,  -803, -1605, -2404, -3196, -3980, -4756, -5519, -6269, -7005, -7723, -8423, -9102, -9759,-10393,-11002,-11585,-12139,-12665,-13159,-13622,-14053,-14449,-14810,-15136,-15426,-15678,-15892,-16069,-16206,-16305,-16364,-16384,-16364,-16305,-16206,-16069,-15892,-15678,-15426,-15136,-14810,-14449,-14053,-13622,-13159,-12665,-12139,-11585,-11002,-10393, -9759, -9102, -8423, -7723, -7005, -6269, -5519, -4756, -3980, -3196, -2404, -1605,  -803,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, },
{      0,  -402,  -803, -1205, -1605, -2005, -2404, -2801, -3196, -3589, -3980, -4369, -4756, -5139, -5519, -5896, -6269, -6639, -7005, -7366, -7723, -8075, -8423, -8765, -9102, -9434, -9759,-10079,-10393,-10701,-11002,-11297,-11585,-11866,-12139,-12406,-12665,-12916,-13159,-13395,-13622,-13842,-14053,-14255,-14449,-14634,-14810,-14978,-15136,-15286,-15426,-15557,-15678,-15790,-15892,-15985,-16069,-16142,-16206,-16260,-16305,-16339,-16364,-16379,-16384,-16379,-16364,-16339,-16305,-16260,-16206,-16142,-16069,-15985,-15892,-15790,-15678,-15557,-15426,-15286,-15136,-14978,-14810,-14634,-14449,-14255,-14053,-13842,-13622,-13395,-13159,-12916,-12665,-12406,-12139,-11866,-11585,-11297,-11002,-10701,-10393,-10079, -9759, -9434, -9102, -8765, -8423, -8075, -7723, -7366, -7005, -6639, -6269, -5896, -5519, -5139, -4756, -4369, -3980, -3589, -3196, -2801, -2404, -2005, -1605, -1205,  -803,  -402, },
};

const unsigned char sqrtbl[]={
	  0,   1,   1,   1,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
	  4,   4,   4,   4,   4,   4,   4,   4,   4,   5,   5,   5,   5,   5,   5,   5,
	  5,   5,   5,   5,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
	  6,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
	  8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
	  8,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
	  9,   9,   9,   9,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
	 10,  10,  10,  10,  10,  10,  10,  10,  10,  11,  11,  11,  11,  11,  11,  11,
	 11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
	 12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
	 12,  12,  12,  12,  12,  12,  12,  12,  12,  13,  13,  13,  13,  13,  13,  13,
	 13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
	 13,  13,  13,  13,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
	 14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
	 14,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
	 15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
	};
//---------- proto
int pgaInit();
void (*pga_channel_callback[PGA_CHANNELS])(void *buf, unsigned long reqn);
static int pga_channel_thread(int args, void *argp);
void pga_channel_thread_callback(int channel, void *buf, unsigned long reqn);
void pgaSetChannelCallback(int channel, void *callback);
int pgaOutBlocking(unsigned long channel,unsigned long vol1,unsigned long vol2,void *buf);

static void wavout_snd0_callback(short *_buf, unsigned long _reqn);
static void wavout_snd1_callback(short *_buf, unsigned long _reqn);
void powercalc(short *in);
unsigned long sqri(unsigned long d);
int wavoutLoadWav(const char *filename, wavout_wavinfo_t *wi, void *buf, unsigned long buflen);

int pgfOpen(const char *filename, unsigned long flag);
void pgfClose(int fd);
int pgfRead(int fd, void *data, int size);

void wavoutStopPlay0();
void wavoutStartPlay0(wavout_wavinfo_t *wi);

//---------- function
int pgaInit()
{
	int i,ret;
	int failed=0;
	char str[32];

	pga_terminate=0;
	pga_ready=0;

	for (i=0; i<PGA_CHANNELS; i++) {
		pga_handle[i]=-1;
		pga_threadhandle[i]=-1;
		pga_channel_callback[i]=0;
	}
	for (i=0; i<PGA_CHANNELS; i++) {
		if ((pga_handle[i]=sceAudio_3(-1,PGA_SAMPLES,0))<0) failed=1;
	}
	if (failed) {
		for (i=0; i<PGA_CHANNELS; i++) {
			if (pga_handle[i]!=-1) sceAudio_4(pga_handle[i]);
			pga_handle[i]=-1;
		}
		return -1;
	}
	pga_ready=1;

	strcpy(str,"pgasnd0");
	for (i=0; i<PGA_CHANNELS; i++) {
		str[6]='0'+i;
		pga_threadhandle[i]=sceKernelCreateThread(str,(pg_threadfunc_t)&pga_channel_thread,0x12,0x10000,0,NULL);
		if (pga_threadhandle[i]<0) {
			pga_threadhandle[i]=-1;
			failed=1;
			break;
		}
		ret=sceKernelStartThread(pga_threadhandle[i],sizeof(i),&i);
		if (ret!=0) {
			failed=1;
			break;
		}
	}
	if (failed) {
		pga_terminate=1;
		for (i=0; i<PGA_CHANNELS; i++) {
			if (pga_threadhandle[i]!=-1) {
				sceKernelWaitThreadEnd(pga_threadhandle[i],NULL);
				sceKernelDeleteThread(pga_threadhandle[i]);
			}
			pga_threadhandle[i]=-1;
		}
		pga_ready=0;
		return -1;
	}
	return 0;
}

static int pga_channel_thread(int args, void *argp)
{
	volatile int bufidx=0;
	int channel=*(int *)argp;
	
	while (pga_terminate==0) {
		void *bufptr=&pga_sndbuf[channel][bufidx];
		void (*callback)(void *buf, unsigned long reqn);
		callback=pga_channel_callback[channel];
		if (callback) {
			callback(bufptr,PGA_SAMPLES);
		} else {
			unsigned long *ptr=bufptr;
			int i;
			for (i=0; i<PGA_SAMPLES; ++i) *(ptr++)=0;
		}
		pgaOutBlocking(channel,0x8000,0x8000,bufptr);
		bufidx=(bufidx?0:1);
	}
	sceKernelExitThread(0);
	return 0;
}


void pga_channel_thread_callback(int channel, void *buf, unsigned long reqn)
{
	void (*callback)(void *buf, unsigned long reqn);
	callback=pga_channel_callback[channel];
}


void pgaSetChannelCallback(int channel, void *callback)
{
	pga_channel_callback[channel]=callback;
}

int pgaOutBlocking(unsigned long channel,unsigned long vol1,unsigned long vol2,void *buf)
{
	if (!pga_ready) return -1;
	if (channel>=PGA_CHANNELS) return -1;
	if (vol1>MAXVOLUME) vol1=MAXVOLUME;
	if (vol2>MAXVOLUME) vol2=MAXVOLUME;
	return sceAudio_2(pga_handle[channel],vol1,vol2,buf);
}

int wavoutInit()
{
	int i;
	
	wavout_snd0_wavinfo=0;
	
	for (i=0; i<SND1_MAXSLOT; i++) {
		wavout_snd1_playing[i]=0;
	}

	pgaSetChannelCallback(0,wavout_snd0_callback);
	pgaSetChannelCallback(1,wavout_snd1_callback);
	return 0;
}

static void wavout_snd0_callback(short *_buf, unsigned long _reqn)
{
	static int power[128];
	
	unsigned long i;
	unsigned long ptr,frac,rr,max;
	int channels;
	char *src;
	short *buf=_buf;
	unsigned long reqn=_reqn;
	
	wavout_wavinfo_t *wi=wavout_snd0_wavinfo;

	if (wi==0) {
		wavout_snd0_ready=1;
		memset(buf,0,reqn*4);
		return;
	}
	
	wavout_snd0_ready=0;
	
	ptr=wi->playptr;
	frac=wi->playptr_frac;
	rr=wi->rateratio;
	max=wi->samplecount;
	channels=wi->channels;
	src=wi->wavdata;

	for (; reqn>0; --reqn) {
		frac+=rr;
		ptr+=(frac>>16);
		frac&=0xffff;
		if (ptr>=max) {
			if (wi->playloop) {
				ptr=0;
			} else {
				for (; reqn>0; --reqn) {
					*(buf++)=0;
					*(buf++)=0;
				}
				goto playend;
			}
		}
		if (channels==1) {
			buf[0]=buf[1]=*(short *)(src+ptr*2);
			buf+=2;
		} else {
			buf[0]=*(short *)(src+ptr*4);
			buf[1]=*(short *)(src+ptr*4+2);
			buf+=2;
		}
	}

	powercalc(_buf);	//単にwaveを出すだけなら不要

	wavout_snd0_playptr=ptr;
	wi->playptr=ptr;
	wi->playptr_frac=frac;
	return;
	
playend:
	wavout_snd0_playend=1;
	return;
}

static void wavout_snd1_callback(short *_buf, unsigned long _reqn)
{
	unsigned long i,slot;
	wavout_wavinfo_t *wi;
	unsigned long ptr,frac;
	short *buf=_buf;
	
	for (i=0; i<_reqn; i++) {
		int outr=0,outl=0;
		for (slot=0; slot<SND1_MAXSLOT; slot++) {
			if (!wavout_snd1_playing[slot]) continue;
			wi=&wavout_snd1_wavinfo[slot];
			frac=wi->playptr_frac+wi->rateratio;
			wi->playptr=ptr=wi->playptr+(frac>>16);
			wi->playptr_frac=(frac&0xffff);
			if (ptr>=wi->samplecount) {
				wavout_snd1_playing[slot]=0;
				break;
			}
			short *src=(short *)wi->wavdata;
			if (wi->channels==1) {
				outl+=src[ptr];
				outr+=src[ptr];
			} else {
				outl+=src[ptr*2];
				outr+=src[ptr*2+1];
			}
		}
		if (outl<-32768) outl=-32768;
		else if (outl>32767) outl=32767;
		if (outr<-32768) outr=-32768;
		else if (outr>32767) outr=32767;
		*(buf++)=outl;
		*(buf++)=outr;
	}
}

unsigned long sqri(unsigned long d)
{
	unsigned char c;
	unsigned long r;
	
	
	if (d==0) return 0;
	r=	(c=sqrtbl[((unsigned char *)&d)[3]])?((unsigned long)c<<12):(
		(c=sqrtbl[((unsigned char *)&d)[2]])?((unsigned long)c<<8):(
		(c=sqrtbl[((unsigned char *)&d)[1]])?((unsigned long)c<<4):(
		sqrtbl[((unsigned char *)&d)[0]] )));
	r=(d/r+r)>>1;
	r=(d/r+r)>>1;
	r=(d/r+r)>>1;
	r=(d/r+r)>>1;
	return r;

}

void powercalc(short *in)
{
	{//work with sources
		int i,j,sum;
		for (i=0; i<256-16; i++) powersrc[i]=powersrc[i+16];
		sum=0;
		for (i=0; i<16; i++) {
			powersrc[256-16+i]=( (int)in[i*32]+(int)in[i*32+8]+(int)in[i*32+16]+(int)in[i*32+24] )/4;
		}
	}
	{
		int i;
		for (i=0; i<256; i++) powersrc2[i]=powersrc[i];
		for (i=0; i<32; i++) {
			powersrc2[i]=((int)powersrc2[i])*i/32;
			powersrc2[255-i]=((int)powersrc2[255-i])*i/32;
		}
	}
	
	long m=8;
	long n=256;

	long m1,m2;

	long i,i1,j,k,i2,l,l1,l2;

	int ix[256],iy[256];
	int tx,ty,pw;

	//shuffle
	for (i=0; i<256; i++) {
		ix[i]=(int)(powersrc2[brtbl[i]]>>1);
		iy[i]=0;
	}

	//fft main
	l2 = 1;
	for (l=0; l<8; l++) {
		l1 = l2;
		l2 <<= 1;
		for (j=0; j<l1; j++) {
			m1=mm1[l][j]/64;
			m2=mm2[l][j]/64;
			for (i=j; i<n; i+=l2) {
				i1 = i + l1;
				tx = ( m1 * ix[i1] - m2 * iy[i1] )/256;
				ty = ( m1 * iy[i1] + m2 * ix[i1] )/256;
				ix[i1] = ix[i] - tx; 
				iy[i1] = iy[i] - ty;
				ix[i] += tx;
				iy[i] += ty;
			}
		}
	}

	//scale & normalize
	short *pp=powerbuf[powerbuf_in];
	for (i=0; i<128; i++) {
		//tx,ty : re,im  forward fft scaling=256, fixed point +-16384
		tx=ix[i]/256;
		ty=iy[i]/256;
		pw=(sqri(tx*tx+ty*ty));
		//normalize it 
		pw=pw*(j+8)/512;
		if (pw>127) pw=127;
		*(pp++)=pw;
	}
	powerbuf_in=((powerbuf_in+1)&(PB_N-1));

   return;
}

int wavoutLoadWav(const char *filename, wavout_wavinfo_t *wi, void *buf, unsigned long buflen)
{
	unsigned int filelen;
	int fd;
	unsigned long channels;
	unsigned long samplerate;
	unsigned long blocksize;
	unsigned long bitpersample;
	unsigned long datalength;
	unsigned long samplecount;
	unsigned long i;
	
	char *wavfile=buf;
	wi->wavdata=NULL;

	fd=pgfOpen(filename,O_RDONLY);
	if (fd<0) return -1;
	
	filelen=pgfRead(fd,wavfile,buflen);
	pgfClose(fd);
	if (filelen>=buflen) {
		//too long
		return -1;
	}
	
	if (memcmp(wavfile,"RIFF",4)!=0) {
//		pgcPuts("format err");
		return -1;
	}
	
	if (memcmp(wavfile+8,"WAVEfmt \x10\x00\x00\x00\x01\x00",14)!=0) {
//		pgcPuts("format err");
		return -1;
	}
	
	channels=*(short *)(wavfile+0x16);
	samplerate=*(long *)(wavfile+0x18);
	blocksize=*(short *)(wavfile+0x20);
	bitpersample=*(short *)(wavfile+0x22);
	
	if (memcmp(wavfile+0x24,"data",4)!=0) {
//		pgcPuts("format err");
		return -1;
	}
	
	datalength=*(unsigned long *)(wavfile+0x28);
	
	if (datalength+0x2c>filelen) {
//		pgcPuts("format err");
		return -1;
	}
	
	if (channels!=2 && channels!=1) {
//		pgcPuts("format err, channel");
		return -1;
	}
	
//	if (samplerate!=44100 && samplerate!=22050 && samplerate!=11025) {
	if (samplerate>100000 || samplerate<2000) {
//		pgcPuts("format err, samplerate");
		return -1;
	}
	
	if (blocksize!=channels*2) {
//		pgcPuts("format err, blocksize");
		return -1;
	}
	
	if (bitpersample!=16) {
//		pgcPuts("format err, bitpersample");
		return -1;
	}
	
	if (channels==2) {
		samplecount=datalength/4;
	} else {
		samplecount=datalength/2;
	}
	if (samplecount<=0) {
//		pgcPuts("format err, samplecount");
		return -1;
	}
	
	wi->channels=channels;
	wi->samplerate=samplerate;
	wi->samplecount=samplecount;
	wi->datalength=datalength;
	wi->wavdata=wavfile+0x2c;
	wi->rateratio=(samplerate*0x4000)/11025;
	wi->playptr=0;
	wi->playptr_frac=0;
	wi->playloop=0;
	
	return 0;
}

int pgfOpen(const char *filename, unsigned long flag)
{
	char fn[MAX_PATH*2];
	if (strchr(filename,':')!=NULL || *filename=='/' || *filename=='\\') {
		return sceIoOpen(filename,flag);
	} else {
		strcpy(fn,pg_workdir);
		strcat(fn,filename);
		return sceIoOpen(fn,flag);
	}
}

void pgfClose(int fd)
{
	sceIoClose(fd);
}

int pgfRead(int fd, void *data, int size)
{
	return sceIoRead(fd,data,size);
}

void wavoutStopPlay0()
{
	if (wavout_snd0_wavinfo!=0) {
		while (wavout_snd0_ready) sceDisplayWaitVblankStart();;
		wavout_snd0_wavinfo=0;
		while (!wavout_snd0_ready) sceDisplayWaitVblankStart();;
	}
}

void wavoutStartPlay0(wavout_wavinfo_t *wi)
{
	wavoutStopPlay0();
	while (!wavout_snd0_ready) sceDisplayWaitVblankStart();;
	wavout_snd0_playptr=0;
	wavout_snd0_playend=0;
	wavout_snd0_wavinfo=wi;
	while (wavout_snd0_ready) sceDisplayWaitVblankStart();;
}

