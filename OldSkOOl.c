// AloneTrio intro2 sample
// www.psp.to

#include "home_button.c"

#include "gfx_top.c"
#include "gfx_bottom.c"
#include "gfx_amiga.c"
#include "font8x8.c"
#include "pspfdf.c"

#include "pg.c"

#include "scrolltxtintro.c"

#include "sound.c"

#define NUMSTARS 900
#define MAX_PATH 512



typedef struct {
	unsigned long x; 
	unsigned long y; 
	unsigned long speed; 
	unsigned long pos; 
} star;



static star tabstars[NUMSTARS]; 
//char *pg_vramtop=(char *)0x04000000;
unsigned char *vptr; 
char pg_mypath[MAX_PATH];
char pg_workdir[MAX_PATH];

//---------------------------------------------------- Main Code
void xmain(unsigned long args, void *argp)
{
	unsigned long i,j,k,x,y=0,starcol;
	unsigned long ypos;
	int ret;
	int n;
	unsigned long scrptr=0,sinptr=0,oldsinptr=0,sinrand=0;
    unsigned long trux1=0,shit1=0,trux2=0,shit2=0,trux3=0,shit3=0;
    float ang1=0,ang2=0,tempx=0,tempy=0,tempz=0;
	unsigned long tabpointx[14],tabpointy[14],oldx[3][14],oldy[3][14];
	unsigned short frame=0;
	
	SetupCallbacks();  // Init Home Button
	
//---------------------------------------------------- Init I/O et Sound
    n=args;
	if (n>sizeof(pg_mypath)-1) n=sizeof(pg_mypath)-1;
	memcpy(pg_mypath,argp,n);
	pg_mypath[sizeof(pg_mypath)-1]=0;
	strcpy(pg_workdir,pg_mypath);
	for (n=strlen(pg_workdir); n>0 && pg_workdir[n-1]!='/'; --n) pg_workdir[n-1]=0;
	pgaInit();
	wavoutInit();
	wavoutLoadWav("data.dat",&wavinfo_bg,wavdata_bg,sizeof(wavdata_bg));
	wavinfo_bg.playloop=1;

//---------------------------------------------------- Init Video
	pgInit();
	pgScreenFrame(2,0);
    cls();
	
//---------------------------------------------------- ecran AMIGA	
	pgFillvram(0x7fff);
	
	sceDisplayWaitVblankStart();
	PutGfx((480-amiga_width)/2,(272-amiga_high)/2,amiga_width,amiga_high,1,amiga_img);
	
	pgScreenFlipV();
	pgWaitVn(60*3);
    cls();
	   
//----------------------------------------  STARS et logo -----------------------------------
	for (i=0; i<NUMSTARS; i++)
	{
		tabstars[i].x = _rand(480);
		tabstars[i].y = _rand(272-top_high-(bottom_high+1+11))+top_high;
		tabstars[i].speed = (_rand(3))+1;
		tabstars[i].pos = 0;
	}
	
	for(k=0;k<14;k++){
	  oldx[0][k]=0;
	  oldy[0][k]=0;
  	  oldx[1][k]=0;
	  oldy[1][k]=0;
	  oldx[2][k]=0;
	  oldy[2][k]=0;
	}
	
	cls();
	
	wavoutStartPlay0(&wavinfo_bg);	
	
	PutGfx(0,0,top_width,top_high,1,top_img);
	PutGfx(0,260,bottom_width,bottom_high,1,bottom_img);
	pgScreenFlipV();
	PutGfx(0,0,top_width,top_high,1,top_img);
	PutGfx(0,260,bottom_width,bottom_high,1,bottom_img);
		
	while(1){
	// 3d------------
	ang1+=0.02f;
	  if(ang1>=6.483185307f) ang1=0;
	  ang2+=0.01f;
	  if(ang2>=6.483185307f) ang2=0;

      for(k=0;k<14;k++){
        tempz=-pspfdfx[k]*(trigfunc(2,3,(ang1))*trigfunc(2,3,(ang2)))-pspfdfy[k]*(trigfunc(1,3,(ang1))*trigfunc(2,3,(ang2)))-pspfdfz[k]*trigfunc(1,3,(ang2))+340;
        tempx= (-pspfdfx[k]*trigfunc(1,3,(ang1))+pspfdfy[k]*trigfunc(2,3,(ang1)))/tempz;
        tempy= (-pspfdfx[k]*(trigfunc(2,3,(ang1))*trigfunc(1,3,(ang2)))-pspfdfy[k]*(trigfunc(1,3,(ang1))*trigfunc(1,3,(ang2)))+pspfdfz[k]*trigfunc(2,3,(ang2)))/tempz;
        tabpointx[k]=tempx*200+240;
        tabpointy[k]=tempy*200+176;
        oldx[frame][k]=tabpointx[k];
        oldy[frame][k]=tabpointy[k];
      }
      frame++;
      if(frame==3) frame=0;
      
      pgLine(oldx[frame][0],oldy[frame][0],oldx[frame][1],oldy[frame][1],0);
      pgLine(oldx[frame][1],oldy[frame][1],oldx[frame][2],oldy[frame][2],0);
      pgLine(oldx[frame][2],oldy[frame][2],oldx[frame][3],oldy[frame][3],0);
      pgLine(oldx[frame][3],oldy[frame][3],oldx[frame][4],oldy[frame][4],0);
      
      pgLine(oldx[frame][5],oldy[frame][5],oldx[frame][6],oldy[frame][6],0);
      pgLine(oldx[frame][6],oldy[frame][6],oldx[frame][7],oldy[frame][7],0);
      pgLine(oldx[frame][7],oldy[frame][7],oldx[frame][8],oldy[frame][8],0);
      
      pgLine(oldx[frame][9],oldy[frame][9],oldx[frame][10],oldy[frame][10],0);
      pgLine(oldx[frame][10],oldy[frame][10],oldx[frame][11],oldy[frame][11],0);
      pgLine(oldx[frame][11],oldy[frame][11],oldx[frame][12],oldy[frame][12],0);
      pgLine(oldx[frame][12],oldy[frame][12],oldx[frame][13],oldy[frame][13],0);
      
	
	//---------------- STARS -----------------------	
		for (i=0; i<NUMSTARS; i++)
		{
			vptr = pgGetVramAddrLog(0,0)+tabstars[i].pos-(tabstars[i].speed)*2*2 ;
			*(unsigned short *)vptr = 0;
			
			ypos = tabstars[i].y;
			ypos = ypos*512*2; 
			
			tabstars[i].x += tabstars[i].speed; 
			
			if (tabstars[i].x > 489)
				tabstars[i].x = 0;
			
			ypos += (tabstars[i].x)*2; 
			
			
			if(tabstars[i].speed==1) starcol=0x56b5;
			if(tabstars[i].speed==2) starcol=0x6f7b;
			if(tabstars[i].speed==3) starcol=0x7fff; 
			vptr = pgGetVramAddr(0,0) + ypos ;
			*(unsigned short *)vptr= starcol;
			tabstars[i].pos = ypos; 
		}
		
      pgLine(tabpointx[0],tabpointy[0],tabpointx[1],tabpointy[1],0x7fff);
      pgLine(tabpointx[1],tabpointy[1],tabpointx[2],tabpointy[2],0x7fff);
      pgLine(tabpointx[2],tabpointy[2],tabpointx[3],tabpointy[3],0x7fff);
      pgLine(tabpointx[3],tabpointy[3],tabpointx[4],tabpointy[4],0x7fff);
      
      pgLine(tabpointx[5],tabpointy[5],tabpointx[6],tabpointy[6],0x7fff);
      pgLine(tabpointx[6],tabpointy[6],tabpointx[7],tabpointy[7],0x7fff);
      pgLine(tabpointx[7],tabpointy[7],tabpointx[8],tabpointy[8],0x7fff);
      
      pgLine(tabpointx[9],tabpointy[9],tabpointx[10],tabpointy[10],0x7fff);
      pgLine(tabpointx[10],tabpointy[10],tabpointx[11],tabpointy[11],0x7fff);
      pgLine(tabpointx[11],tabpointy[11],tabpointx[12],tabpointy[12],0x7fff);
      pgLine(tabpointx[12],tabpointy[12],tabpointx[13],tabpointy[13],0x7fff);
      
	
	//---------------------------- Scroll ------------------------------
      i=0-trux1;
       for(j=1;j<=61;j++){
	    scrptr=scrolltxtintro_txt[shit1+j];
	    for(x=0;x<font8x8_char_width;x++){
	      for(y=0;y<font8x8_char_high;y++){
	        if(font8x8_font[scrptr]==0) trux2=0;
	        else{
	          if((i==0) || (i==479)) trux2=0x421;
	          if((i==1) || (i==478)) trux2=0xc63;
	          if((i==2) || (i==477)) trux2=0x1ce7;
	          if((i==3) || (i==476)) trux2=0x2529;
	          if((i==4) || (i==475)) trux2=0x2d6b;
	          if((i==5) || (i==474)) trux2=0x35ad;
	          if((i==6) || (i==473)) trux2=0x3def;
	          if((i==7) || (i==472)) trux2=0x4631;
	          if((i==8) || (i==471)) trux2=0x4e73;
	          if((i==9) || (i==470)) trux2=0x56b5;
	          if((i==10) || (i==469)) trux2=0x5ef7;
	          if((i==11) || (i==468)) trux2=0x6739;
	          if((i==12) || (i==467)) trux2=0x6f7b;
	          if((i==13) || (i==466)) trux2=0x77bd;
	          ;
	          
			  if((i>13) && (i<466)) trux2=0x7fff;

	        }
	        PutPixel(i,y+264,trux2);
			scrptr++;
          } 
          i=i+1;
        }
       }
     trux1+=1;
     if(trux1>=font8x8_char_width){
       trux1=0;
       shit1++;
       if(shit1>scrolltxtintro_lenght-60){
         shit1=0;
       }
      }
      //----------------
      
     pgScreenFlipV();
  }
//---------------------------------------------------- Fin STARS et LOGO ----------------------------
}
