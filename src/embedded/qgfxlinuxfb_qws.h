/****************************************************************************
**
** Implementation of QGfxRaster (unaccelerated graphics context) class for.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGFXLINUXFB_QWS_H
#define QGFXLINUXFB_QWS_H

#ifndef QT_H
#include "qgfx_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_LINUXFB

class QLinuxFb_Shared
{
public:
    volatile int lastop;
    volatile int optype;
    volatile int fifocount;   // Accel drivers only
    volatile int fifomax;
    volatile int forecol;     // Foreground colour cacheing
    volatile unsigned int buffer_offset;   // Destination
    volatile int linestep;
    volatile int cliptop;    // Clip rectangle
    volatile int clipleft;
    volatile int clipright;
    volatile int clipbottom;
    volatile unsigned int rop;

};

class QLinuxFbScreen : public QScreen
{
public:
    QLinuxFbScreen( int display_id );
    virtual ~QLinuxFbScreen();

    virtual bool initDevice();
    virtual bool connect( const QString &displaySpec );

    virtual bool useOffscreen() { return FALSE; }

    virtual void disconnect();
    virtual void shutdownDevice();
    virtual void setMode(int,int,int);
    virtual void save();
    virtual void restore();
    virtual void blank(bool on);
    virtual void set(unsigned int,unsigned int,unsigned int,unsigned int);
    virtual uchar * cache(int,int);
    virtual void uncache(uchar *);
    virtual int sharedRamSize(void *);

    QLinuxFb_Shared * shared;

protected:

    void deleteEntry(uchar *);

    bool canaccel;
    int dataoffset;
    int cacheStart;

    static void clearCache( QScreen *instance, int );

private:

    void delete_entry(int);
    void insert_entry(int,int,int);
    void setupOffScreen();

    int fd;
    int startupw;
    int startuph;
    int startupd;
    fb_cmap *startcmap;
};

#endif

#endif // QGFXLINUXFB_QWS_H
