/****************************************************************************
**
** Definition of QStatusBar class.
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

#ifndef QSTATUSBAR_H
#define QSTATUSBAR_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_STATUSBAR


class QStatusBarPrivate;


class Q_GUI_EXPORT QStatusBar: public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool sizeGripEnabled READ isSizeGripEnabled WRITE setSizeGripEnabled )

public:
    QStatusBar( QWidget* parent=0, const char* name=0 );
    virtual ~QStatusBar();

    virtual void addWidget( QWidget *, int stretch = 0, bool = FALSE );
    virtual void removeWidget( QWidget * );

    void setSizeGripEnabled(bool);
    bool isSizeGripEnabled() const;

public slots:
    void message( const QString &);
    void message( const QString &, int );
    void clear();

signals:
    void messageChanged( const QString &text );

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

#endif // QT_NO_STATUSBAR

#endif // QSTATUSBAR_H
