/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BIFF_H
#define BIFF_H

#include <qwidget.h>
#include <qdatetime.h>
#include <qpixmap.h>


class Biff : public QWidget
{
    Q_OBJECT
public:
    Biff( QWidget *parent=0, const char *name=0 );

protected:
    void	timerEvent( QTimerEvent * );
    void	paintEvent( QPaintEvent * );
    void	mousePressEvent( QMouseEvent * );

private:
    QDateTime	lastModified;
    QPixmap	hasNewMail;
    QPixmap	noNewMail;
    QString	mailbox;
    bool	gotMail;
};


#endif // BIFF_H
