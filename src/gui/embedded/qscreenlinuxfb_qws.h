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

#ifndef QSCREENLINUXFB_QWS_H
#define QSCREENLINUXFB_QWS_H

#include "QtGui/qscreen_qws.h"

QT_MODULE(Gui)

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

struct fb_cmap;
struct fb_var_screeninfo;
struct fb_fix_screeninfo;
class QLinuxFbScreen : public QScreen
{
public:
    explicit QLinuxFbScreen(int display_id);
    virtual ~QLinuxFbScreen();

    virtual bool initDevice();
    virtual bool connect(const QString &displaySpec);

    virtual bool useOffscreen() { return false; }

    virtual void disconnect();
    virtual void shutdownDevice();
    virtual void setMode(int,int,int);
    virtual void save();
    virtual void restore();
    virtual void blank(bool on);
    virtual void set(unsigned int,unsigned int,unsigned int,unsigned int);
    virtual uchar * cache(int);
    virtual void uncache(uchar *);
    virtual int sharedRamSize(void *);

    QLinuxFb_Shared * shared;

protected:

    void deleteEntry(uchar *);

    bool canaccel;
    int dataoffset;
    int cacheStart;

    static void clearCache(QScreen *instance, int);

private:

    void delete_entry(int);
    void insert_entry(int,int,int);
    void setupOffScreen();
    void createPalette(fb_cmap &cmap, fb_var_screeninfo &vinfo, fb_fix_screeninfo &finfo);

    int fd;
    int startupw;
    int startuph;
    int startupd;
};

#endif

#endif // QSCREENLINUXFB_QWS_H
