/*****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Implementation of QGfxRaster (unaccelerated graphics context) class for
** Embedded Qt
** Created : 940721
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qgfxraster_qws.h"
#include "qmemorymanager_qws.h"
#include "qwsdisplay_qws.h"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "qgfxlinuxfb_qws.h"


// Unaccelerated screen/driver setup. Can be overridden by accelerated
// drivers

QLinuxFbScreen::QLinuxFbScreen( int display_id ) : QScreen( display_id )
{
}

QLinuxFbScreen::~QLinuxFbScreen()
{
}

bool QLinuxFbScreen::connect( const QString &displaySpec )
{
    QString dev( "/dev/fb0" );

    // Check for explicitly specified device
    QRegExp r( "/dev/.*:" );
    int len;
    int m = r.match( displaySpec, 0, &len );
    if ( m >= 0 ) {
	dev = displaySpec.mid( m, len-1 );
    }

    //qDebug( "QLinuxFbScreen: using device %s", dev.latin1() );

    fd=open( dev.latin1(), O_RDWR );
    if(fd<0) {
	perror("openning framebuffer device /dev/fb0");
	qFatal("Can't open framebuffer device");
    }

    fb_fix_screeninfo finfo;
    fb_var_screeninfo vinfo;

    /* Get fixed screen information */
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo)) {
	perror("reading /dev/fb0");
	qFatal("Error reading fixed information");
    }

    /* Get variable screen information */
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
	perror("reading /dev/fb0");
	qFatal("Error reading variable information");
    }

    const char* qwssize;
    if((qwssize=getenv("QWS_SIZE"))) {
	sscanf(qwssize,"%dx%d",&w,&h);
    } else {
	w=vinfo.xres;
	h=vinfo.yres;
    }
    d=vinfo.bits_per_pixel;
    lstep=(vinfo.xres_virtual*d+7)/8;
    //qDebug("Using %dx%dx%d screen",w,h,d);

    /* Figure out the size of the screen in bytes */
    size = h * lstep;

    mapsize=finfo.smem_len;

    data = (unsigned char *)mmap(0, mapsize, PROT_READ | PROT_WRITE,
				 MAP_SHARED, fd, 0);
    if ((int)data == -1) {
	perror("mapping /dev/fb0");
	qFatal("Error: failed to map framebuffer device to memory.");
    }

    // Now read in palette
    if(vinfo.bits_per_pixel==8) {
	screencols=256;
	unsigned int loopc;
	startcmap = new fb_cmap;
	startcmap->start=0;
	startcmap->len=256;
	startcmap->red=(unsigned short int *)
		 malloc(sizeof(unsigned short int)*256);
	startcmap->green=(unsigned short int *)
		   malloc(sizeof(unsigned short int)*256);
	startcmap->blue=(unsigned short int *)
		  malloc(sizeof(unsigned short int)*256);
	startcmap->transp=(unsigned short int *)
		    malloc(sizeof(unsigned short int)*256);
	ioctl(fd,FBIOGETCMAP,startcmap);
	for(loopc=0;loopc<256;loopc++) {
	    screenclut[loopc]=qRgb(startcmap->red[loopc] >> 8,
				   startcmap->green[loopc] >> 8,
				   startcmap->blue[loopc] >> 8);
	}
    } else {
	screencols=0;
    }

    // No blankin' screen, no blinkin' cursor!
    const char termctl[] = "\033[9;0]\033[?33l";
    write(1,termctl,sizeof(termctl));

    initted=true;

    return TRUE;
}

void QLinuxFbScreen::disconnect()
{
    munmap((char*)data,mapsize);
    close(fd);
    // a reasonable screensaver timeout
    printf( "\033[9;15]" );
    fflush( stdout );
}

bool QLinuxFbScreen::initCard()
{
    // Grab current mode so we can reset it
    fb_var_screeninfo vinfo;
    fb_fix_screeninfo finfo;

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
	qFatal("Error reading variable information in card init");
	return false;
    }

    startupw=vinfo.xres;
    startuph=vinfo.yres;
    startupd=vinfo.bits_per_pixel;

    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo)) {
	qFatal("Error reading fixed information in card init");
	// It's not an /error/ as such, though definitely a bad sign
	// so we return true
	return true;
    }

#ifdef __i386__
    // Now init mtrr
    if(!getenv("QWS_NOMTRR")) {
	int mfd=open("/proc/mtrr",O_WRONLY,0);
	// MTRR entry goes away when file is closed - i.e.
	// hopefully when QWS is killed
	if(mfd==-1) {
	    // /proc/mtrr not writable - oh well.
	} else {
	    mtrr_sentry sentry;
	    sentry.base=(unsigned long int)finfo.smem_start;
	    qDebug("Physical framebuffer address %08lx",finfo.smem_start);
	    // Size needs to be in 4k chunks, but that's not always
	    // what we get thanks to graphics card registers. Write combining
	    // these is Not Good, so we write combine what we can
	    // (which is not much - 4 megs on an 8 meg card, it seems)
	    unsigned int size=finfo.smem_len;
	    size=size >> 22;
	    size=size << 22;
	    sentry.size=size;
	    sentry.type=MTRR_TYPE_WRCOMB;
	    if(ioctl(mfd,MTRRIOC_ADD_ENTRY,&sentry)==-1) {
		printf("Couldn't add mtrr entry for %lx %lx, %s\n",
		       sentry.base,sentry.size,strerror(errno));
	    }
	}
    }
#endif

    if(vinfo.bits_per_pixel==8) {
	screencols=256;
	fb_cmap cmap;
	cmap.start=0;
	cmap.len=256;
	cmap.red=(unsigned short int *)
		 malloc(sizeof(unsigned short int)*256);
	cmap.green=(unsigned short int *)
		   malloc(sizeof(unsigned short int)*256);
	cmap.blue=(unsigned short int *)
		  malloc(sizeof(unsigned short int)*256);
	cmap.transp=(unsigned short int *)
		    malloc(sizeof(unsigned short int)*256);
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
	// Build greyscale palette
	unsigned int loopc;
	for(loopc=0;loopc<256;loopc++) {
	    cmap.red[loopc]=loopc << 8;
	    cmap.green[loopc]=loopc << 8;
	    cmap.blue[loopc]=loopc << 8;
	    cmap.transp[loopc]=0;
	    screenclut[loopc]=qRgb(loopc,loopc,loopc);
	}
#else
	// 6x6x6 216 color cube
	int idx = 0;
	for( int ir = 0x0; ir <= 0xff; ir+=0x33 ) {
	    for( int ig = 0x0; ig <= 0xff; ig+=0x33 ) {
		for( int ib = 0x0; ib <= 0xff; ib+=0x33 ) {
		    cmap.red[idx] = ir << 8;
		    cmap.green[idx] = ig << 8;
		    cmap.blue[idx] = ib << 8;
		    cmap.transp[idx] = 0;
		    screenclut[idx]=qRgb( ir, ig, ib );
		    idx++;
		}
	    }
	}
	// Fill in rest with 0
	for ( int loopc=0; loopc<40; loopc++ ) {
	    screenclut[idx]=0;
	    idx++;
	}
	screencols=idx;
#endif
	ioctl(fd,FBIOPUTCMAP,&cmap);
	free(cmap.red);
	free(cmap.green);
	free(cmap.blue);
	free(cmap.transp);
    } else if(finfo.visual==FB_VISUAL_DIRECTCOLOR) {
	// This code is just base on what the cyber2000 driver expects
	screencols=256;
	fb_cmap cmap;
	cmap.start=0;
	int rbits,gbits,bbits;
	switch (vinfo.bits_per_pixel) { 
	  case 8:
		rbits=3;
		gbits=3;
		bbits=2;
		break;
	  case 15:
		rbits=5;
		gbits=5;
		bbits=5;
		break;
	  case 16:
		rbits=5;
		gbits=6;
		bbits=5;
		break;
	  case 24: case 32:
		rbits=gbits=bbits=8;
		break;
	}
	cmap.len=1<<QMAX(rbits,QMAX(gbits,bbits));
	cmap.red=(unsigned short int *)
		 malloc(sizeof(unsigned short int)*256);
	cmap.green=(unsigned short int *)
		   malloc(sizeof(unsigned short int)*256);
	cmap.blue=(unsigned short int *)
		  malloc(sizeof(unsigned short int)*256);
	cmap.transp=(unsigned short int *)
		    malloc(sizeof(unsigned short int)*256);
	for( int i = 0x0; i < cmap.len; i++ ) {
	    cmap.red[i] = i*65535/((1<<rbits)-1);
	    cmap.green[i] = i*65535/((1<<gbits)-1);
	    cmap.blue[i] = i*65535/((1<<bbits)-1);
	    cmap.transp[i] = 0;
	}
	ioctl(fd,FBIOPUTCMAP,&cmap);
	free(cmap.red);
	free(cmap.green);
	free(cmap.blue);
	free(cmap.transp);
    }

    initted=true;

    return true;
}

void QLinuxFbScreen::shutdownCard()
{
    // Set back the original mode
#ifndef QT_NO_QWS_CURSOR
    if ( qt_sw_cursor )
	qt_screencursor->hide();
#endif

    // Causing crashes. Not needed.
    //setMode(startupw,startuph,startupd);
/*
    if ( startupd == 8 ) {
	ioctl(fd,FBIOPUTCMAP,startcmap);
	free(startcmap->red);
	free(startcmap->green);
	free(startcmap->blue);
	free(startcmap->transp);
	delete startcmap;
	startcmap = 0;
    }
*/
}


void QLinuxFbScreen::set(unsigned int i,unsigned int r,unsigned int g,unsigned int b)
{
    fb_cmap cmap;
    cmap.start=i;
    cmap.len=1;
    cmap.red=(unsigned short int *)
	     malloc(sizeof(unsigned short int)*256);
    cmap.green=(unsigned short int *)
	       malloc(sizeof(unsigned short int)*256);
    cmap.blue=(unsigned short int *)
	      malloc(sizeof(unsigned short int)*256);
    cmap.transp=(unsigned short int *)
		malloc(sizeof(unsigned short int)*256);
    cmap.red[0]=r << 8;
    cmap.green[0]=g << 8;
    cmap.blue[0]=b << 8;
    cmap.transp[0]=0;
    ioctl(fd,FBIOPUTCMAP,&cmap);
    free(cmap.red);
    free(cmap.green);
    free(cmap.blue);
    free(cmap.transp);
}


void QLinuxFbScreen::setMode(int nw,int nh,int nd)
{
    fb_var_screeninfo vinfo;

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
	qFatal("Error reading variable information in mode change");
    }

    vinfo.xres=nw;
    vinfo.yres=nh;
    vinfo.bits_per_pixel=nd;

    if (ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo)) {
	qFatal("Error writing variable information in mode change");
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
	qFatal("Error reading changed variable information in mode change");
    }

    w=vinfo.xres;
    h=vinfo.yres;
    d=vinfo.bits_per_pixel;
    lstep=(vinfo.xres_virtual*d+7)/8;
    size=w * h * d / 8;
}

// save the state of the graphics card
// This is needed so that e.g. we can restore the palette when switching
// between linux virtual consoles.
void QLinuxFbScreen::save()
{
    // nothing to do.
}

// restore the state of the graphics card.
void QLinuxFbScreen::restore()
{
    if ( d == 8 ) {
	fb_cmap cmap;
	cmap.start=0;
	cmap.len=screencols;
	cmap.red=(unsigned short int *)
		 malloc(sizeof(unsigned short int)*256);
	cmap.green=(unsigned short int *)
		   malloc(sizeof(unsigned short int)*256);
	cmap.blue=(unsigned short int *)
		  malloc(sizeof(unsigned short int)*256);
	cmap.transp=(unsigned short int *)
		    malloc(sizeof(unsigned short int)*256);
	for ( int loopc = 0; loopc < screencols; loopc++ ) {
	    cmap.red[loopc] = qRed( screenclut[loopc] ) << 8;
	    cmap.green[loopc] = qGreen( screenclut[loopc] ) << 8;
	    cmap.blue[loopc] = qBlue( screenclut[loopc] ) << 8;
	    cmap.transp[loopc] = 0;
	}
	ioctl(fd,FBIOPUTCMAP,&cmap);
	free(cmap.red);
	free(cmap.green);
	free(cmap.blue);
	free(cmap.transp);
    }
}


extern "C" QScreen * qt_get_screen_linuxfb(int display_id, const char *spec,
					   char *,unsigned char *)
{
    if ( !qt_screen ) {
	const char *term = getenv( "TERM" );
	if ( QString( term ).left(5) == "xterm" ) {
	    qFatal( "$TERM=xterm - To continue would corrupt X11 - aborting" );
	}
	qt_screen=new QLinuxFbScreen( display_id );
	qt_screen->connect( spec );
    }
    return qt_screen;
}


