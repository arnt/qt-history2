/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
****************************************************************************/

// Mach64 card initialisation, done by QWS server

// Takes pointer to /proc/bus/pci/xxx

#ifndef _QWSMACH64_
#define _QWSMACH64_

#include "qwsaccel.h"
#include "qwsmach64defs.h"

#include <sys/io.h>

class Mach64Accel : public AccelCard {

public:

    Mach64Accel(unsigned char *,unsigned char *);
    virtual ~Mach64Accel();

    virtual void vesa_init(int,int,int);

    virtual void cursor_enabled(bool);
    virtual void move_cursor(int,int);

private:

    unsigned int vgabase;
    unsigned char * membase;

    void setup_cursor();

    unsigned int regr(unsigned int);
    void regw(unsigned int,unsigned long);
    unsigned int ioportr(unsigned short);
    void ioportw(unsigned short,unsigned int);
    void wait_for_fifo(short);
    void wait_for_idle();
    void reset_engine();

};

inline unsigned int Mach64Accel::regr(volatile unsigned int regindex)
{
    unsigned long int val;
    unsigned long temp=(unsigned long)regbase;
    temp+=regindex;
    val=*((volatile unsigned long *)(temp));
    return val;
}

inline void Mach64Accel::regw(volatile unsigned int regindex,
			      unsigned long val)
{
    unsigned long temp;
    temp=(unsigned long)regbase;
    temp+=regindex;
    *((volatile unsigned long int *)(temp))=val;
}

inline unsigned int Mach64Accel::ioportr(unsigned short port)
{
    return regr(port);
    /*
    port+=vgabase;
    unsigned int ret;
    printf("IO port read from %x\n",port);
    ret=inl(port);
    return ret;
    */
}

inline void Mach64Accel::ioportw(unsigned short port,unsigned int val)
{
    regw(port,val);
    /*
    port+=vgabase;
    printf("IO port write to %x\n",port);
    outl(port,val);
    */
}

inline void Mach64Accel::wait_for_fifo(short entries)
{
    //if(entries<16)
    //entries++;
    //printf("Waiting for %d, fifo state %d\n",entries,
    //       regr(FIFO_STAT) & 0xffff);

    for(;;) {
	int wizzy=regr(FIFO_STAT);
	if(wizzy & 0x80000000) {
	    //qDebug("Resetting engine");
	    reset_engine();
	    return;
	}
	//qDebug("FIFO_STAT looks like %x",wizzy);
	wizzy=wizzy & 0xffff;
	int loopc;
	int count=0;
	for(loopc=0;loopc<16;loopc++) {
	    if(!(wizzy & 0x1))
		count++;
	    wizzy=wizzy >> 1;
	}
	//qDebug("Free entries %d need %d",count,entries);
	if(count>=entries)
	    return;
    }
}

inline void Mach64Accel::wait_for_idle()
{
    wait_for_fifo(16);
    //unsigned long rr=regr(GUI_STAT);
    //printf("Reg now %x fifos %d scissor bits %d %d %d %d engine active %d\n",
    //       rr, (rr >> 16) & 0x3f, (rr >> 11) & 1,
    //       (rr >> 10) & 1,(rr >> 9) & 1,(rr >> 8) & 1, rr & 1);
    // page 149
    while((regr(GUI_STAT) & 1)!=0);
}

inline void Mach64Accel::reset_engine()
{
    wait_for_fifo(1);
    ioportw(GEN_TEST_CNTL,(ioportr(GEN_TEST_CNTL) & 0xfffffeff));
    wait_for_fifo(1);
    ioportw(GEN_TEST_CNTL,(ioportr(GEN_TEST_CNTL) | 0x00000100));
    wait_for_fifo(1);
    ioportw(BUS_CNTL,ioportr(BUS_CNTL) | 0x00a00000);
    //ioportw(BUS_CNTL,(ioportr(BUS_CNTL) | BUS_HOST_ERR_ACK |
    //               BUS_FIFO_ERR_ACK));
}

#endif
