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

#ifndef QWSCURSOR_QWS_H
#define QWSCURSOR_QWS_H

#include <QtGui/qimage.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QWSCursor
{
public:
    QWSCursor() {}
    QWSCursor(const uchar *data, const uchar *mask, int width, int height,
              int hotX, int hotY)
        { set(data, mask, width, height, hotX, hotY); }

    void set(const uchar *data, const uchar *mask,
             int width, int height, int hotX, int hotY);

    QPoint hotSpot() const { return hot; }
    QImage &image() { return cursor; }
    const QRegion region() const { return rgn; }

    static QWSCursor *systemCursor(int id);

private:
    static void createSystemCursor(int id);
    void createDropShadow(int dropx, int dropy);

private:
    QPoint hot;
    QImage cursor;
    QRegion rgn;
};

QT_END_HEADER

#endif // QWSCURSOR_QWS_H
