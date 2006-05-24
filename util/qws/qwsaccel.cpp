/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
****************************************************************************/

#include "qwsaccel.h"
#include "qwsmach64.h"
#include <stdio.h>

AccelCard::AccelCard(unsigned char * pcifile,unsigned char * pciinfo)
{
    procentry=pcifile;
}

AccelCard::~AccelCard()
{
    free(procentry);
}

// We may or may not want proper bus probing later

AccelCard * probed_card=0;

static char * bus_pos="/proc/bus/pci/01/00.0";

static bool unaccelerated = false;

static void check_file(char * file,int w,int h,int d)
{
    FILE * f=fopen(file,"r");
    if(!f) {
	// Doesn't exist
	qDebug("No bus entry %s",file);
	return;
    }
    unsigned char config[257];
    int loopc;
    for(loopc=0;loopc<256 && !feof(f);loopc++) {
	config[loopc]=fgetc(f);
    }
    fclose(f);
    unsigned short vendorid=*((unsigned short int *)config);
    unsigned short deviceid=*((unsigned short int *)config+2);
    if(config[0xb]==3) {
	if(vendorid==0x1002) {
		probed_card=new Mach64Accel((unsigned char*)file,config);
		if(!probed_card->inited) {
		    delete probed_card;
		    probed_card=0;
		} else {
		    probed_card->vendorid=vendorid;
		    probed_card->deviceid=deviceid;
		    probed_card->vesa_init(w,h,d);
		}
	} else {
	    unaccelerated = true;
	    return;
	}
    }
}

void probe_bus(int w,int h,int d)
{
    if(probed_card||unaccelerated)
	return;
    check_file(bus_pos,w,h,d);
}

