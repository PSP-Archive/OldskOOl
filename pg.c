// primitive graphics for Hello World PSP

#include "pg.h"
#include "font.c"

//system call
void pspDisplayWaitVblankStart();
void pspDisplaySetMode(long,long,long);
void pspDisplaySetFrameBuf(char *topaddr,long linesize,long pixelsize,long);

#define CMAX_X 60
#define CMAX_Y 38
#define CMAX2_X 30
#define CMAX2_Y 19
#define CMAX4_X 15
#define CMAX4_Y 9

//variables
char *pg_vramtop=(char *)0x04000000;
long pg_screenmode;
long pg_showframe;
long pg_drawframe;



void pgWaitVn(unsigned long count)
{
	for (; count>0; --count) {
		sceDisplayWaitVblankStart();
	}
}


void pgWaitV()
{
	sceDisplayWaitVblankStart();
}


char *pgGetVramAddr(unsigned long x,unsigned long y)
{
	return pg_vramtop+(pg_drawframe?0x44000:0)+x*2+y*512*2+0x40000000;
}

char *pgGetVramAddrLog(unsigned long x,unsigned long y)
{
	return pg_vramtop+(pg_drawframe?0:0x44000)+x*2+y*512*2+0x40000000;
}

void pgInit()
{
	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);
	pgScreenFrame(0,0);
}




void pgFillvram(unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for (i=0; i<0x44000/2; i++) {
		*(unsigned short *)vptr0=color;
		vptr0+=2;
	}
}


void pgScreenFrame(long mode,long frame)
{
	pg_screenmode=mode;
	frame=(frame?1:0);
	pg_showframe=frame;
	if (mode==0) {
		//screen off
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(0,0,0,1);
	} else if (mode==1) {
		//show/draw same
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?0x44000:0),512,1,1);
	} else if (mode==2) {
		//show/draw different
		pg_drawframe=(frame?0:1);
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?0x44000:0),512,1,1);
	}
}


void pgScreenFlip()
{
	pg_showframe=(pg_showframe?0:1);
	pg_drawframe=(pg_drawframe?0:1);
	sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?0x44000:0),512,1,0);
}


void pgScreenFlipV()
{
	pgWaitV();
	pgScreenFlip();
}



void PutPixel(unsigned long x, unsigned long y, unsigned short color)
{
	unsigned char *vptr;		//pointer to vram
    
    if(x>479)
      x=479;
    if(x<0)
      x=0;
    if(y>271)
      y=271;
    if(y<0)
      y=0;
      
	vptr=pgGetVramAddr(x,y);
	*(unsigned short *)vptr=color;
}
void PutGfx(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	const unsigned short *dd;
	
	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
					*(unsigned short *)vptr=*dd;
					vptr+=2;
				}
				dd++;
			}
			vptr0+=512*2;
		}
		d+=w;
	}
	
}

void PutGfxTrans(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	const unsigned short *dd;
	
	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
				    if(*dd!=0){
					*(unsigned short *)vptr=*dd;
					}
					vptr+=2;
				}
				dd++;
			}
			vptr0+=512*2;
		}
		d+=w;
	}
	
}

void FilledRec(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	
	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			 for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
				    
					*(unsigned short *)vptr=color;
					
					vptr+=2;
				}
			}
			vptr0+=512*2;
		}
	}
}

//**-------------------
void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar(x*8,y*8,color,0,*str,1,0,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}


void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

	if (ch>255) return;
	cfont=font+ch*8;
	vptr0=pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((*cfont&b)!=0) {
						if (drawfg) *(unsigned short *)vptr=color;
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
					}
					vptr+=1*2;
				}
				b=b>>1;
			}
			vptr0+=512*2;
		}
		cfont++;
	}
}

void cls()
{
      pgFillvram(0);
	  pgScreenFlip();
      pgFillvram(0);

}

// generate 7 transcendental values with one function
// p = function selector,  #define the following:
// SIN 1   COS 2   TAN 3   EXP 4   SINH 5   COSH 6   TANH 7
// n = convergence cutoff, n = 3 is typical,
// higher n gives higher precision, but slower execution
//
float trigfunc(int p, int n, float x)
{
        int     k;
        float  r, s, t;

        if (p <= 3)
        {
                r = - x * x;    // trig
        }
        else
        {
                r = x * x;      // hyperbolic
        }
        s = 4 * n + 2;
        for (k = n; k > 0; k--)
        {
                s = 4 * k - 2 + r/s;
        }
        switch (p % 4)
        {
                case 0 : t = (s + x)/(s - x);         // exp
                break;
                case 1 : t = 2 * x * s/(s * s - r);   // sin, sinh
                break;
                case 2 : t = (s * s + r)/(s * s - r); // cos, cosh
                break;
                case 3 : t = 2 * x * s/(s * s + r);   // tan, tanh
                break;
        }
        return (t);
}

unsigned _rand(unsigned long MAX)
{
	Seed1 = (Seed1 + 46906481) ^ Seed2;
	Seed2 = Seed1 ^ ( ((Seed2<<3) | (Seed2 >> 29)) + 103995407);
   
	return (Seed1 - Seed2)%MAX;
} 

pgLine(float x1, float y1, float x2, float y2, unsigned long index)
{
   float indice;
   float dx,dy;
   float x,y;
   float m;
     if(x1==x2)
     x2++;
     if(x1>x2)
     indice=1;
     else
     indice=0;
  
     if (!indice)
     {
         dx=x2-x1;
         dy=y2-y1;
         m=(float)dy/dx;
         if(m<=1&&m>=-1)
         {
             for(x=0; x<dx; x++)
             {
                 y=m*x+y1;
                 PutPixel(x+x1, y, index);
             }
         }
         else
         {
             if(dx==1&&y2>y1)
             {
                 for(;y1<y2;y1++)
                     PutPixel(x1,y1,index);
             }
             else
             if(dx==1&&y2<y1)
             {
                 for(;y1>y2;y1--)
                     PutPixel(x1,y1,index);
             }
             else
             {
                 m=(float)dx/dy;
                 if(m>0)
                 {
                     for(y=0;y<=dy;y++)
                     {
                         x=m*y+x1;
                         PutPixel(x,y+y1,index);
                     }
                 }
                 else
                 {
                     for(y=0;y>dy;y--)
                     {
                         x=m*y+x1;
                         PutPixel(x,y+y1,index);
                     }
                 }
             }
             }
         }
     if (indice==1)
     {
         dx=x1-x2;
         dy=y1-y2;
         m=(float)dy/dx;
         if(m<=1&&m>=-1)
         {
             for(x=0;x<dx;x++)
             {
                 y=y1-m*x;
                 PutPixel(x1-x, y, index);
             }
         }
         else
         {
             m=(float)dx/dy;
             if(m>0)
             {
                 for(y=0;y<dy;y++)
                 {
                     x=x1-m*y;
                     PutPixel(x,y1-y,index);
                 }
             }
             else
             {
                 for(y=0;y>dy;y--)
                 {
                     x=x1-m*y;
                     PutPixel(x,y1-y,index);
                 }
             }
         }
     }
}
