/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindow.h#16 $
**
** Definition of QWSCursor class
**
** Created : 000101
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
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
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWSCURSOR_H
#define QWSCURSOR_H

#ifndef QT_H
#include <qimage.h>
#endif // QT_H

class QWSCursor
{
public:
    QWSCursor() {}
    QWSCursor(const uchar *data, const uchar *mask,
                int width, int height, int hotX, int hotY)
	{ set(data, mask, width, height, hotX, hotY); }

    void set(const uchar *data, const uchar *mask,
		int width, int height, int hotX, int hotY);

    QPoint hotSpot() const { return hot; }
    QImage &image() { return cursor; }
    const QRegion region() const { return rgn; }

    static QWSCursor *systemCursor(int id);

private:
    static void createSystemCursor( int id );	
    void createDropShadow(int dropx, int dropy);

private:
    QPoint hot;
    QImage cursor;
    QRegion rgn;
};



#endif // QWSCURSOR_H
