/****************************************************************************
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


#ifndef QHBOX_H
#define QHBOX_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_HBOX

#include "qframe.h"

class QBoxLayout;

class Q_EXPORT QHBox : public QFrame
{
    Q_OBJECT
public:
    QHBox( QWidget* parent=0, const char* name=0, WFlags f=0 );

    void setSpacing( int );
    bool setStretchFactor( QWidget*, int stretch );
    QSize sizeHint() const;

protected:
    QHBox( bool horizontal, QWidget* parent=0, const char* name=0, WFlags f = 0 );
    void frameChanged();

private:
    QBoxLayout *lay;

#if defined(Q_DISABLE_COPY)
    QHBox( const QHBox & );
    QHBox &operator=( const QHBox & );
#endif
};

#endif // QT_NO_HBOX

#endif // QHBOX_H
