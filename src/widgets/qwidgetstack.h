/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qwidgetstack.h#21 $
**
** Definition of QWidgetStack class
**
** Created : 980306
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWIDGETSTACK_H
#define QWIDGETSTACK_H

#ifndef QT_H
#include "qframe.h"
#include "qintdict.h"
#include "qptrdict.h"
#endif // QT_H

#if QT_FEATURE_WIDGETS


class QWidgetStackPrivate;


class Q_EXPORT QWidgetStack: public QFrame
{
    Q_OBJECT
public:
    QWidgetStack( QWidget * parent = 0, const char *name = 0 );
    ~QWidgetStack();

    void addWidget( QWidget *, int );
    void removeWidget( QWidget * );

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void show();

    QWidget * widget( int ) const;
    int id( QWidget * ) const;

    QWidget * visibleWidget() const;

    void setFrameRect( const QRect & );

signals:
    void aboutToShow( int );
    void aboutToShow( QWidget * );

public slots:
    void raiseWidget( int );
    void raiseWidget( QWidget * );

protected:
    void frameChanged();
    void resizeEvent( QResizeEvent * );

    virtual void setChildGeometries();
    void childEvent( QChildEvent * );

private:
    bool isMyChild( QWidget * );

    QWidgetStackPrivate * d;
    QIntDict<QWidget> * dict;
    QPtrDict<QWidget> * focusWidgets;
    QWidget * topWidget;
    QWidget * invisible;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWidgetStack( const QWidgetStack & );
    QWidgetStack& operator=( const QWidgetStack & );
#endif
};

#endif // QT_FEATURE_WIDGETS

#endif // QWIDGETSTACK_H
