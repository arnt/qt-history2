/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qradiobutton.h#38 $
**
** Definition of QRadioButton class
**
** Created : 940222
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QRADIOBUTTON_H
#define QRADIOBUTTON_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H


class Q_EXPORT QRadioButton : public QButton
{
    Q_OBJECT
public:
    QRadioButton( QWidget *parent, const char *name=0 );
    QRadioButton( const QString &text, QWidget *parent, const char* name=0 );

    bool    isChecked() const;
    virtual void    setChecked( bool check );

    QSize    sizeHint() const;
    QSizePolicy sizePolicy() const;

protected:
    bool    hitButton( const QPoint & ) const;
    void    drawButton( QPainter * );
    void    drawButtonLabel( QPainter * );

    void    resizeEvent( QResizeEvent* );

    void    updateMask();

private:
    void    init();
    uint    noHit : 1;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QRadioButton( const QRadioButton & );
    QRadioButton &operator=( const QRadioButton & );
#endif
};


inline bool QRadioButton::isChecked() const
{ return isOn(); }

inline void QRadioButton::setChecked( bool check )
{ setOn( check ); }


#endif // QRADIOBUTTON_H
