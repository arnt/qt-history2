/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qstatusbar.h#9 $
**
** Definition of QStatusBar class
**
** Created : 980316
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSTATUSBAR_H
#define QSTATUSBAR_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class QStatusBarPrivate;


class Q_EXPORT QStatusBar: public QWidget
{
    Q_OBJECT
public:
    QStatusBar( QWidget * parent = 0, const char *name = 0 );
    ~QStatusBar();

    void addWidget( QWidget *, int stretch = 0, bool = FALSE );
    void removeWidget( QWidget * );

    void setSizeGripEnabled(bool);
    bool sizeGripEnabled() const;

public slots:
    void message( const QString &);
    void message( const QString &, int );
    void clear();

protected:
    void paintEvent( QPaintEvent * );

    void reformat();
    void hideOrShow();

private:
    QStatusBarPrivate * d;
};


#endif
