/****************************************************************************
**
** Definition of QWidgetStack class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWIDGETSTACK_H
#define QWIDGETSTACK_H

#ifndef QT_H
#include "qframe.h"
#include "qintdict.h"
#endif // QT_H

#ifndef QT_NO_WIDGETSTACK


class QWidgetStackPrivate;


class Q_GUI_EXPORT QWidgetStack: public QFrame
{
    Q_OBJECT
public:
    QWidgetStack(QWidget* parent=0, const char* name=0, WFlags f = 0);

    ~QWidgetStack();

    int addWidget( QWidget *, int = -1 );
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
    bool event( QEvent * );

    QWidgetStackPrivate * d;
    QIntDict<QWidget> * dict;
    QWidget * topWidget;
    QWidget * invisible;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWidgetStack( const QWidgetStack & );
    QWidgetStack& operator=( const QWidgetStack & );
#endif
};

#endif // QT_NO_WIDGETSTACK

#endif // QWIDGETSTACK_H
