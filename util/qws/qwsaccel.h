#ifndef _QWSACCEL_
#define _QWSACCEL_

#include "qglobal.h"

class AccelCard {

public:

    AccelCard(unsigned char *,unsigned char *);
    virtual ~AccelCard();

    virtual void vesa_init(int,int,int) {}

    virtual void cursor_enabled(bool) {}
    virtual void move_cursor(int,int) {}

    unsigned char * regbase;
    unsigned int vendorid;
    unsigned int deviceid;
    unsigned char * procentry;

    bool inited;

};

extern AccelCard * probed_card;

void probe_bus(int,int,int);

#endif
