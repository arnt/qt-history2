/****************************************************************************
** $Id: .emacs,v 1.3 1998/02/20 15:06:53 agulbra Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997-2005 by Trolltech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QWIDGET_QWS_P_H
#define QWIDGET_QWS_P_H

#include <qbytearray.h>

class QWSBackingStore
{
public:
    QWSBackingStore();
    ~QWSBackingStore();

    void create(QSize size);
    void attach(int shmid, QSize size);
    void detach();

    void lock(bool write=false);
    void unlock();

    QPixmap *pixmap();



    int memoryId() const { return shmid; }
    QSize size() const;

private:
    QPixmap *pix;
    int shmid;
    void *shmaddr;
};
#endif
