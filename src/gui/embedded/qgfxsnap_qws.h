/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
**
** Definition of QSNAPScreen and QGfxSNAP classes for Embedded Qt
**
** Copyright (C) SciTech Software, Inc.  All rights reserved.
**
** Author:  Kendall Bennett
**
** This module requires the SciTech SNAP SDK to build and also requires
** the SciTech SNAP runtime device driver modules to be present in order
** to run.
**
** See http://www.scitechsoft.com or email sales@scitechsoft.com for
** information about SciTech SNAP Commercial License Agreements.
**
**********************************************************************/

#ifndef QGFXSNAP_QWS_H
#define QGFXSNAP_QWS_H

#ifndef QT_NO_QWS_SNAP

#include "QtGui/qscreen_qws.h"
#include "snap/graphics.h"
#include "snap/ref2d.h"

/* Forward reference */
struct QGfxSNAP_State;

/*!
\class QSNAPScreen
\ingroup qws
\brief The QSNAPScreen class manages the SciTech SNAP Graphics device driver.

QSNAPScreen is a descendant of QScreen; there is precisely one per Qt/Embedded
application.
*/

class QSNAPScreen : public QScreen
{
public:
    explicit QSNAPScreen( int display_id );
    virtual ~QSNAPScreen();

    virtual void setMode(int,int,int);
    virtual bool connect( const QString &displaySpec );
    virtual int sharedRamSize(void *);
    virtual bool initDevice();
    virtual int initCursor(void *end_of_location,bool init);
    virtual void disconnect();
    virtual void shutdownDevice();
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual void save();
    virtual void restore();
    virtual void blank(bool on);
    virtual void set(unsigned int,unsigned int,unsigned int,unsigned int);
    virtual uchar * cache(int);
    virtual void uncache(uchar *);

protected:
    // Note: These values are reported in *bits* rather than the expected *bytes*!!
    virtual int pixmapOffsetAlignment() { return modeInfo.BitmapStartAlign * 8; }
    virtual int pixmapLinestepAlignment() { return modeInfo.BitmapStridePad * 8; }

private:
    void fatalCleanup();
    N_uint16 findMode(int x,int y,int bits);
    int initSoftwareRasterizer();
    void delete_entry(int);
    void insert_entry(int,uint,uint);
    void setupOffScreen();

private:
    PM_HWND             hwndConsole;
    void                *stateBuf;
    int                 isServer;
    int                 useOffscreen;
    ulong               cacheStart;
    GA_devCtx           *dc;
    GA_initFuncs        init;
    GA_driverFuncs      driver;
    GA_DPMSFuncs        dpms;
    GA_2DStateFuncs     hwState2d;
    GA_2DRenderFuncs    hwDraw2d;
    GA_2DStateFuncs     state2d;
    GA_2DRenderFuncs    draw2d;
    QGfxSNAP_State      *cntState;
    REF2D_driver        *ref2d;
    int                 unloadRef2d;
    N_uint16            prevMode;
    N_uint16            cntMode;
    N_int32             maxMem;
    GA_modeInfo         modeInfo;
    N_int16             DPMSStates;
};

#endif // QT_NO_QWS_SNAP

#endif // QGFXSNAP_QWS_H
