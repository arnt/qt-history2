/*
 *              Copyright (C) 2000  Sony Computer Entertainment Inc.
 *                              All Rights Reserved.
 */

#include "gsos.h"

#include <stdio.h>

#include <assert.h>
#define AssertGsosCommandBufferExecuted() assert(ps2count==0)
    // assert command buffer in gsos is flushed

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>
#include "ps2ioctl.h"
#include <stdlib.h>

#define DRIVERNAMEDEV "/dev/ps2dev"
#define DRIVERNAMEMEM "/dev/ps2mem"
#define GSOS_MAXPACKETSIZE 4*1024*1024

/* I've added some debugging to this library:

   DEBUG_GSOS_NOOP : this will cause gsos to work on non ps2 machines, at least for some
   		     definition of "work".

   DEBUG_GSOS_ACCOUNT : Will account for each packet going to the gsos, and if it finds 
   			inconsistancies it will alert me..

   DEBUG_GSOS_SCREEN_SET : Will display screen settings as they are set/pushed/poped
   
			//Sam
*/

#ifdef DEBUG_GSOS_ACCOUNT
#ifndef DEBUG_GSOS_NOOP
#define DEBUG_GSOS_NOOP
#endif
static int expected_count = 0;
static int instruction_count = 0;
void gsosDebug(const char *foo, ...)
{
    va_list bar;
    va_start(bar, foo);
    vfprintf(stderr, foo, bar);
    va_end(bar);
}
#endif

int ps2devfd = -1 ;      /* file pointer for driver */
int ps2memfd = -1 ;      /* file pointer for driver */
GSOSbit64 *ps2buf = NULL ;    /* DMA buffer */
GSOSbit64 *ps2cur = NULL ;    /* current pointer of DMA buffer */
static int ps2count = 0 ;    /* counter for DMA buffer */
static int cache_w=-1, cache_h=-1; /* cached width and height of screen */

int gsosOpen()
{
#ifdef DEBUG_GSOS_ACCOUNT
    instruction_count = expected_count = 0;
#endif

#ifndef DEBUG_GSOS_NOOP    
    /* open driver */
    ps2devfd = open( DRIVERNAMEDEV, O_RDWR ) ;
    if( ps2devfd < 0 ){
	fprintf (stderr,"gsosOpen: can't open driver(DRIVERNAMEDEV)\n" ) ;
        return -1 ;
    }
    
    /* open driver */
    ps2memfd = open( DRIVERNAMEMEM, O_RDWR ) ;
    if( ps2memfd < 0 ){
	fprintf (stderr,"gsosOpen: can't open driver(DRIVERNAMEMEM)\n" ) ;
        return -1 ;
    }

    /* memory allocation for DMA buffer */
    if( ps2buf == NULL ){
        ps2buf = (GSOSbit64 *)mmap( NULL, GSOS_MAXPACKETSIZE, 
            PROT_READ | PROT_WRITE, MAP_SHARED, ps2memfd, 0 ) ;
        if( ps2buf == NULL ){
            //ErrorF( "gsosOpen: memory allocateion error\n");
	    fprintf (stderr,"gsosOpen: memory allocateion error\n");
            close(ps2devfd) ;
            close(ps2memfd) ;
            return -1 ;
        }
    }
#else
    ps2buf = malloc(GSOS_MAXPACKETSIZE);
#endif    

    ps2count = 0 ;
    ps2cur = ps2buf ;
    return 0 ;
}

int gsosClose()
{
#ifdef DEBUG_GSOS_ACCOUNT
    instruction_count = expected_count = 0;
#endif
    
#ifndef DEBUG_GSOS_NOOP    
    munmap( (void *)ps2buf, GSOS_MAXPACKETSIZE ) ;
    if( ps2devfd >= 0 ){
        close( ps2devfd ) ;
        ps2devfd = -1 ;
    }
    if( ps2memfd >= 0 ){
        close( ps2memfd ) ;
        ps2memfd = -1 ;
    }
#endif    
    return 0 ;
}

struct ps2_screeninfo consoleinfo ;
static int consolesaved = 0;
int gsosSave()
{
    if(ps2devfd == -1)
	return 0;
    
#ifndef DEBUG_GSOS_NOOP
    consolesaved = (ioctl( ps2devfd, PS2IOC_GSCREENINFO, &consoleinfo ) == 0);
    if(consolesaved) {
#ifdef DEBUG_GSOS_SCREEN_SET
	printf("gsosSave(): mode=%d res=%d w=%d h=%d fbp=0x%08x psm=%d ch=%d ctx=%d init=%d\n",
	       consoleinfo.mode, consoleinfo.res, consoleinfo.w, consoleinfo.h, consoleinfo.fbp, consoleinfo.psm,
	       consoleinfo.ch, consoleinfo.ctx, consoleinfo.init);
#endif
	cache_w = consoleinfo.w;
	cache_h = consoleinfo.h;
    }
#else
    consolesaved = 1;
#endif
    return consolesaved;
}
int gsosRestore()
{
    if( consolesaved && ps2devfd >= 0 ) {
#ifdef DEBUG_GSOS_NOOP
	return 1;
#else    
	if((consolesaved = (ioctl( ps2devfd, PS2IOC_SSCREENINFO, &consoleinfo ) == 0))) {
	    cache_w = cache_h = -1; //invalidate cached values 
#ifdef DEBUG_GSOS_SCREEN_SET
	    printf("gsosRestore(): mode=%d res=%d w=%d h=%d fbp=0x%08x psm=%d ch=%d ctx=%d init=%d\n",
		   consoleinfo.mode, consoleinfo.res, consoleinfo.w, consoleinfo.h, consoleinfo.fbp, consoleinfo.psm,
		   consoleinfo.ch, consoleinfo.ctx, consoleinfo.init);
#endif
	}
#endif	
    }
    return 0;
}

int gsosGetScreenSize(int *w, int *h)
{
#ifndef DEBUG_GSOS_NOOP                
    struct ps2_screeninfo info ;
#endif    
    if(cache_w != -1 && cache_h != -1) {
	*w = cache_w;
	*h = cache_h;
	return 1;
    }
#ifndef DEBUG_GSOS_NOOP            
    if(ioctl( ps2devfd, PS2IOC_GSCREENINFO, &info ) == 0) {
	cache_w = info.w;
	cache_h = info.h;
	return 1;
    }
#endif    
    return 0;
}

void gsosSetScreen( int mode, int res, int w, int h, int fbp, int psm, int ch, int ctx, int init )
{
    struct ps2_screeninfo info ;

    info.mode = mode ;
    info.res = res ;
    info.w = w ;
    info.h = h ;
    info.fbp = fbp ;
    info.psm = psm ;
    info.ch = ch ;
    info.ctx = ctx ;
    info.init = init ;
#ifndef DEBUG_GSOS_NOOP
    ioctl( ps2devfd, PS2IOC_SSCREENINFO, &info ) ;
#endif
    
#ifdef DEBUG_GSOS_SCREEN_SET
    printf("gsosSet(): mode=%d res=%d w=%d h=%d fbp=0x%08x psm=%d ch=%d ctx=%d init=%d\n",
	   info.mode, info.res, info.w, info.h, info.fbp, info.psm, info.ch, info.ctx, info.init);
#endif    
    cache_w = cache_h = -1; //invalidate cache
}

int gsosMakeGiftag(
    GSOSbit64 nloop,	/* repeat num */
	GSOSbit64 eop,		/* end of packet */
	GSOSbit64 pre,		/* prim field enable */
	GSOSbit64 prim,		/* prim date set to PRIM register */
	GSOSbit64 flg,		/* 0:PACKED 1:REGLIST 2:IMAGE */
	GSOSbit64 nreg,		/* number of regs(MAX=16) */
	GSOSbit64 regs)	/* register description */
{
#ifdef DEBUG_GSOS_ACCOUNT
    int total_count = nloop * nreg; 
    if(!expected_count)
	gsosDebug("gsosMakeGiftag: I'm expecting %d!\n", total_count);
    else
	gsosDebug("gsosMakeGiftag: Ok, you are adding %d to %d, better keep up!\n", total_count, expected_count);
    expected_count += total_count;
#endif
    
    *ps2cur++ = nloop | (eop<<15) | (pre<<46) | (prim<<47)
           | (flg<<58) | (nreg<<60) ;
    *ps2cur++ = regs ;
    ps2count += 2;
    return 0 ;
}

void gsosSetPacketAddrData(
    GSOSbit64 addr, GSOSbit64 data)
{
#ifdef DEBUG_GSOS_ACCOUNT   
    instruction_count++;
    gsosDebug("gsosSetPacketAddrData: that's %d down, %d to go!\n", instruction_count,expected_count-instruction_count);
#endif

    *ps2cur++ = data ;
    *ps2cur++ = addr ;
    ps2count += 2;
}

void gsosSetPacket2(
    GSOSbit64 d1, GSOSbit64 d2)
{
#ifdef DEBUG_GSOS_ACCOUNT   
    instruction_count++;
    gsosDebug("gsosSetPacket2: that's %d down, %d to go!\n", instruction_count,expected_count-instruction_count);
#endif
    
    *ps2cur++ = d1 ;
    *ps2cur++ = d2 ;
    ps2count += 2;
}

void gsosSetPacket4(
    GSOSbit64 d1, GSOSbit64 d2, GSOSbit64 d3, GSOSbit64 d4)
{
#ifdef DEBUG_GSOS_ACCOUNT   
    instruction_count++;
    gsosDebug("gsosSetPacket4: that's %d down, %d to go!\n", instruction_count,expected_count-instruction_count);
#endif
    
    *ps2cur++ = d1 | (d2<<32) ;
    *ps2cur++ = d3 | (d4<<32) ;
    ps2count += 2;
}

void gsosSetPacketAddrData4(
    int addr, int d1, int d2, int d3, int d4)
{
#ifdef DEBUG_GSOS_ACCOUNT   
    instruction_count++;
    gsosDebug("gsosSetPacketAddrData4: that's %d down, %d to go!\n", instruction_count,expected_count-instruction_count);    
#endif
    
    *ps2cur++ = (GSOSbit64)d1 | ((GSOSbit64)d2 << 16)
           | ((GSOSbit64)d3 << 32) | ((GSOSbit64)d4 << 48) ;
    *ps2cur++ = (GSOSbit64)addr ;
    ps2count += 2;
}
    
void gsosExec()
{
    struct ps2_packet pkt ;

#ifdef DEBUG_GSOS_ACCOUNT   
    if(instruction_count != expected_count) {
	gsosDebug("Woop! Wooop! Braindead alert! (%d %d)\n", instruction_count, expected_count);
    } else {
	gsosDebug("Whooo! Just made it!\n");
    }
    instruction_count = expected_count = 0;
#endif
    
    if( ps2count ){
        pkt.ptr = ps2buf ;
        pkt.len = (int)((char *)ps2cur - (char *)ps2buf) ;
#ifndef DEBUG_GSOS_NOOP    	
        ioctl( ps2devfd, PS2IOC_GIFPACKET, &pkt ) ;
#endif
    }
    ps2cur = ps2buf ;
    ps2count = 0 ;
}

void gsosFlush()
{
}

int gsosReadImage( 
	int x, int y, int w, int h,
    unsigned int bp,
    int bw, int psm,
	GSOSuchar *pPix )
{
    struct ps2_image img ;
    size_t size ;

    assert (ps2count == 0);

    img.ptr = ps2buf ;
    img.fbp = bp ;
    img.fbw = bw ;
    img.psm = psm ;
    img.x = x ;
    img.y = y ;
    img.w = w ;
    img.h = h ;

#ifndef DEBUG_GSOS_NOOP    	
#undef NONBLOCK
#ifdef NONBLOCK
#define WAITTIME 100000
    if (ioctl( ps2devfd, PS2IOC_STOREIMAGEA, &img ) < 0) return -1 ;
    {
        unsigned long t = 0 ;
        while( !(ioctl( ps2devfd, PS2IOC_GETEVENT, 0) & PS2EV_VIF1PACKET )){
            t++ ;
            if( !(t % WAITTIME) ){
	        fprintf (stderr,"gsosReadImage: try again %ld\n", t ) ;
                ioctl( ps2devfd, PS2IOC_STOREIMAGEA, &img ) ;
                //gsosClose() ;
                //exit(-1) ;
            }
        }
    }
#else
    if (ioctl( ps2devfd, PS2IOC_STOREIMAGE, &img ) < 0) return -1 ;
#endif
#endif
    
    if( psm == 2 ) {
        size = w * h * 2 ;
    }else{
        size = w * h * 4 ;
    }
    memcpy( (void *)pPix, (void *)ps2buf, size ) ;

    return 0 ;
}

int gsosWriteImage( 
	int x, int y, int w, int h,
    unsigned int bp,
    int bw, int psm,
	GSOSuchar *pPix )
{
    struct ps2_image img ;
    size_t size ;

    assert (ps2count == 0);

    if( psm == 2 ) {
        size = w * h * 2 ;
    }else{
        size = w * h * 4 ;
    }
    memcpy( (void *)ps2buf, (void *)pPix, size ) ;

    img.ptr = ps2buf ;
    img.fbp = bp ;
    img.fbw = bw ;
    img.psm = psm ;
    img.x = x ;
    img.y = y ;
    img.w = w ;
    img.h = h ;
#ifndef DEBUG_GSOS_NOOP    	
    if (ioctl( ps2devfd, PS2IOC_LOADIMAGE, &img ) < 0) return -1 ;
#endif    

    return 0 ;
}

