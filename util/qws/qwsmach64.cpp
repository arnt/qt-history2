/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
****************************************************************************/

#include "qwsmach64.h"
#include "qwsmach64defs.h"
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "qimage.h"
#include "qcolor.h"
#include <stdlib.h>
#include <asm/mtrr.h>

Mach64Accel::Mach64Accel(unsigned char * pcifile,unsigned char * config)
    : AccelCard(pcifile,config)
{
    inited=false;
    if(iopl(3)) {
	qDebug("Can't iopl");
	return;
    }
    unsigned char * bar=config+0x10;
    unsigned long int * addr=(unsigned long int *)bar;
    unsigned long int s=*(addr);
    unsigned long int s2=*(addr+1);
    unsigned long int s3=*(addr+2);
    unsigned long int olds=s;
    if(s2 & 0x1) {
	s2=(s2 >> 1) << 1;
	vgabase=s2;
    } else {
	qFatal("Couldn't find IO space");
    }
    if(s & 0x1) {
	qFatal("First memory area is IO space - not right");
	return;
    } else {
	s=olds;
	int aperturefd;
	aperturefd=open("/dev/mem",O_RDWR);
	if(aperturefd==-1) {
	    printf("Can't open /dev/mem\n");
	    return;
	}
	// Make it on a page boundary or this mmap will fall over
	s=(s >> 12) << 12;
	membase=(unsigned char *)mmap(0,8388608,PROT_READ | PROT_WRITE,
			     MAP_SHARED,aperturefd,s);
	if(membase==0 || ((unsigned long)membase)==-1) {
	    qDebug("Membase mmap error, bye!");
	    return;
	}
	regbase=membase+0x7ffc00;
	regw(BUS_CNTL,regr(BUS_CNTL) | 0x10);
	regbase=0;
	// Now init mtrr
	if(getenv("QWS_MTRR")) {
	    int mfd=open("/proc/mtrr",O_WRONLY,0);
	    // MTRR entry goes away when file is closed - i.e.
	    // hopefully when QWS is killed
	    if(mfd==-1) {
		printf("No /proc/mtrr, sorry\n");
	    } else {
		mtrr_sentry sentry;
		sentry.base=s;
		sentry.size=8388608; // Assume 8mb fb
		sentry.type=MTRR_TYPE_WRCOMB;
		if(ioctl(mfd,MTRRIOC_ADD_ENTRY,&sentry)==-1) {
		    printf("Couldn't add mtrr entry\n");
		}
	    }
	}
	inited=true;
    }
    if(s & 0x1) {
	qFatal("Third memory area is IO space - not right");
	return;
    } else {
	int aperturefd;
	aperturefd=open("/dev/mem",O_RDWR);
	if(aperturefd==-1) {
	    printf("Can't open /dev/mem\n");
	    return;
	}
	// Make it on a page boundary or this mmap will fall over
	s3=(s3 >> 12) << 12;
	regbase=(unsigned char *)mmap(0,4096,PROT_READ | PROT_WRITE,
			     MAP_SHARED,aperturefd,s3);
	regbase=regbase+1024;
    }
}

Mach64Accel::~Mach64Accel()
{
}

void Mach64Accel::vesa_init(int width,int height,int depth)
{
    int xres=width;
    int yres=height;
    int pitch_value=width;

    // Lots of boilerplate from ATI manual, with some extra
    // from XFree Mach64 driver for good measure at the end

    reset_engine();
    ioportw(MEM_VGA_WP_SEL,0x00010000);
    ioportw(MEM_VGA_RP_SEL,0x00010000);

    wait_for_fifo(7);
    regw(CONTEXT_MASK,0xffffffff);
    regw(DST_OFF_PITCH, (pitch_value / 8) << 22);
    regw(DST_Y_X,0);
    regw(DST_HEIGHT,0);
    regw(DST_BRES_ERR,0);
    regw(DST_BRES_INC,0);
    regw(DST_BRES_DEC,0);
    wait_for_fifo(6);
    regw(SRC_OFF_PITCH, (pitch_value / 8 ) << 22);
    regw(SRC_Y_X,0);
    regw(SRC_HEIGHT1_WIDTH1,1);
    regw(SRC_Y_X_START,0);
    regw(SRC_HEIGHT2_WIDTH2,1);
    regw(SRC_CNTL,SRC_LINE_X_LEFT_TO_RIGHT);
    wait_for_fifo(13);
    regw(HOST_CNTL,0);
    regw(PAT_REG0,0);
    regw(PAT_REG1,0);
    regw(PAT_CNTL,0);
    regw(SC_LEFT,0);
    regw(SC_TOP,0);
    regw(SC_BOTTOM,yres-1);
    regw(SC_RIGHT,pitch_value-1);
    regw(DP_BKGD_CLR,0);
    regw(DP_FRGD_CLR,0xffffffff);
    regw(DP_WRITE_MASK,0xffffffff);
    regw(DP_SRC,FRGD_SRC_FRGD_CLR);
    wait_for_fifo(3);
    regw(CLR_CMP_CLR,0);
    regw(CLR_CMP_MASK,0xffffffff);
    regw(CLR_CMP_CNTL,0);
    wait_for_fifo(2);
    regw(DP_PIX_WIDTH,HOST_16BPP | SRC_16BPP | DST_16BPP |
	 BYTE_ORDER_LSB_TO_MSB);
    regw(DP_CHAIN_MASK,0x8410);
    wait_for_idle();

    wait_for_fifo(3);
    regw(DST_X,0);
    regw(DST_Y,0);
    regw(DST_HEIGHT,760);
    wait_for_idle();

    wait_for_fifo(5);
    regw(DP_FRGD_CLR,0xffffffff);
    regw(DP_WRITE_MASK,0xffffffff);
    regw(DP_SRC,0x00000100);
    regw(CLR_CMP_CNTL,0x00000000);
    regw(GUI_TRAJ_CNTL,0x00000003);

    wait_for_fifo(9);
    regw(DST_CNTL,0x3);
    regw(DST_BRES_ERR,0);
    regw(DST_BRES_INC,0);
    regw(DST_BRES_DEC,0);
    regw(SRC_Y_X,0);
    regw(SRC_HEIGHT1_WIDTH1,0);
    regw(SRC_Y_X_START,0);
    regw(SRC_HEIGHT2_WIDTH2,0);
    regw(SRC_CNTL,0);

    wait_for_fifo(4);
    regw(HOST_CNTL,regr(HOST_CNTL) & ~HOST_BYTE_ALIGN);
    regw(PAT_REG0,0);
    regw(PAT_REG1,0);
    regw(PAT_CNTL,0);

    wait_for_fifo(7);
    regw(DP_BKGD_CLR,0);
    regw(DP_FRGD_CLR,1);
    regw(DP_MIX,(MIX_SRC << 16) | MIX_DST);
    regw(DP_SRC,FRGD_SRC_FRGD_CLR);
    regw(CLR_CMP_CLR,0);
    regw(CLR_CMP_MASK,0xffffffff);
    regw(CLR_CMP_CNTL,0);

    cursor_enabled(true);
}

bool made_cursor=false;

int gval(QRgb r)
{
    if(qBlue(r)>240) {
	return 0;
    } else if(qBlue(r)<2) {
	return 1;
    } else {
	return 2;
    }
}

void Mach64Accel::setup_cursor()
{
    made_cursor=true;
    QString fn=getenv("HOME");
    fn+="/cursor.bmp";
    QImage i(fn);
    if(i.isNull()) {
	qDebug("Can't find cursor bitmap %s!",fn.ascii());
	return;
    }
    unsigned int offset=(1280*1280*2);
    unsigned char * wibble=membase+offset;
    int loopc,loopc2;
    unsigned char * wibble2=wibble;
    // 3=invert,binary 1==CLR1, 2==nothing(?), 0=CLR0
    for(loopc=0;loopc<64;loopc++) {
	for(loopc2=0;loopc2<16;loopc2++) {
	    unsigned int v1,v2,v3,v4;
	    unsigned int wiggler=loopc2*4;
	    v1=gval(i.pixel(wiggler,loopc));
	    v2=gval(i.pixel(wiggler+1,loopc));
	    v3=gval(i.pixel(wiggler+2,loopc));
	    v4=gval(i.pixel(wiggler+3,loopc));
	    unsigned char put=(v4 << 6) | (v3 << 4) | (v2 << 2) | v1;
	    *(wibble2++)=put;
	}
    }
    wait_for_fifo(5);
    regw(CUR_CLR0,0xffff0000);
    regw(CUR_CLR1,0x44444400);
    regw(CUR_OFFSET,offset/8);
    regw(CUR_HORZ_VERT_OFF,0x00000000);
    regw(CUR_HORZ_VERT_POSN,0x00ff00ff);
}

void Mach64Accel::cursor_enabled(bool on)
{
    if(!made_cursor)
	setup_cursor();

    unsigned int wobble=regr(GEN_TEST_CNTL);
    if(on) {
	wobble=wobble | 0x80;
    } else {
	wobble=wobble & 0xffffff7f;
    }
    wait_for_fifo(1);
    regw(GEN_TEST_CNTL,wobble);
}

void Mach64Accel::move_cursor(int x,int y)
{
    unsigned int hold=x | (y << 16);
    regw(CUR_HORZ_VERT_POSN,hold);
}


