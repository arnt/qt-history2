/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qstatusbar.h#9 $
**
** Definition of QStatusBar class
**
** Created : 980316
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the widgets
** module and therefore may only be used if the widgets module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSTATUSBAR_H
#define QSTATUSBAR_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_COMPLEXWIDGETS


class QStatusBarPrivate;


class Q_EXPORT QStatusBar: public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool sizeGripEnabled READ isSizeGripEnabled WRITE setSizeGripEnabled )
    
public:
    QStatusBar( QWidget * parent = 0, const char *name = 0 );
    ~QStatusBar();

    void addWidget( QWidget *, int stretch = 0, bool = FALSE );
    void removeWidget( QWidget * );

    void setSizeGripEnabled(bool);
    bool isSizeGripEnabled() const;

public slots:
    void message( const QString &);
    void message( const QString &, int );
    void clear();

protected:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );

    void reformat();
    void hideOrShow();
    bool event( QEvent *);
private:
    QStatusBarPrivate * d;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QStatusBar( const QStatusBar & );
    QStatusBar& operator=( const QStatusBar & );
#endif
};

#endif // QT_NO_COMPLEXWIDGETS

#endif // QSTATUSBAR_H
