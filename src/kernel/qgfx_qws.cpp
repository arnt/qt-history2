/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Implementation of QGfx (graphics context) class
**
** Created : 990721
**
** Copyright (C) 1999-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/
#include "qgfx_qws.h"

#include <stdio.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>

bool qt_sw_cursor=false;
QScreenCursor * qt_screencursor=0;
QScreen * qt_screen=0;

QGfx *QGfx::createGfx( int depth, unsigned char *buffer, int w, int h,
			     int offs )
{
    return qt_screen->createGfx( buffer, w, h, depth, offs );
}

#ifdef QT_LOADABLE_MODULES

static QScreen * qt_dodriver(char * driver,char * a,unsigned char * b)

{    
    char buf[200];
    strcpy(buf,"/etc/qws/drivers/");
    strcpy(buf+17,driver);
    qDebug("Attempting driver %s",driver);
    
    void * handle;
    handle=dlopen(buf,RTLD_LAZY);
    if(handle==0) {
	qFatal("Module load error");
    }
    QScreen *(*qt_get_screen_func)(char *,unsigned char *);
    qt_get_screen_func=dlsym(handle,"qt_get_screen");
    if(qt_get_screen_func==0) {
	qFatal("Couldn't get symbol");
    }
    QScreen * ret=qt_get_screen_func(a,b);
    return ret;
}

static QScreen * qt_do_entry(char * entry)
{
    unsigned char config[256];

    FILE * f=fopen(entry,"r");
    if(!f) {
	qDebug("Can't open %s",entry);
	return 0;
    }

    int r=fread(config,256,1,f);
    if(r<1) {
	qDebug("Error reading config information, %d",r);
	return 0;
    }

    fclose(f);
    
    unsigned short vendorid=*((unsigned short int *)config);
    unsigned short deviceid=*(((unsigned short int *)config)+1);
    if(config[0xb]!=3)
	return 0;

    if(vendorid==0x1002) {
	if(deviceid==0x4c4d) {
	    qDebug("Compaq Armada/IBM Thinkpad's Mach64 card");
	    return qt_dodriver("mach64.so",entry,config);
	} else if(deviceid==0x4742) {
	    qDebug("Desktop Rage Pro Mach64 card");
	    return qt_dodriver("mach64.so",entry,config);
	} else {
	    qDebug("Unrecognised ATI card id %x",deviceid);
	    return 0;
	}
    } else {
	qDebug("Unrecognised vendor");
    }
    return 0;
}

extern bool qws_accel;

QScreen * qt_probe_bus()
{
    if(!qws_accel) {
	return qt_dodriver("unaccel.so",0,0);
    }
    
    DIR * dirptr=opendir("/proc/bus/pci");
    if(!dirptr)
	return qt_dodriver("unaccel.so",0,0);
    DIR * dirptr2;
    dirent * cards;

    dirent * busses=readdir(dirptr);

    while(busses) {
	if(busses->d_name[0]!='.') {
	    char buf[100];
	    strcpy(buf,"/proc/bus/pci/");
	    strcpy(buf+14,busses->d_name);
	    int p=strlen(buf);
	    dirptr2=opendir(buf);
	    if(dirptr2) {
		cards=readdir(dirptr2);
		while(cards) {
		    if(cards->d_name[0]!='.') {
			buf[p]='/';
			strcpy(buf+p+1,cards->d_name);
			QScreen * ret=qt_do_entry(buf);
			if(ret)
			    return ret;
		    }
		    cards=readdir(dirptr2);
		}
		closedir(dirptr2);
	    }
	}
	busses=readdir(dirptr);
    }
    closedir(dirptr);

    return qt_dodriver("unaccel.so",0,0);
}

#else

char * hardcoded_slot="/proc/bus/pci/01/00.0";

QScreen * qt_probe_bus()
{
    unsigned char config[256];

    FILE * f=fopen(hardcoded_slot,"r");
    if(!f) {
	qDebug("Can't open %s",hardcoded_slot);
	hardcoded_slot=0;
    } else {
	int r=fread(config,256,1,f);
	if(r<1) {
	    qDebug("Error reading config information, %d",r);
	    hardcoded_slot=0;
	}

	fclose(f);
    }
    
    return qt_get_screen(hardcoded_slot,config);
}

#endif
