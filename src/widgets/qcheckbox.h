/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcheckbox.h#33 $
**
** Definition of QCheckBox class
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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QCHECKBOX_H
#define QCHECKBOX_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H

#if 0
Q_OBJECT
#endif

class Q_EXPORT QCheckBox : public QButton
{
    Q_COMPONENT
public:
    QCheckBox( QWidget *parent, const char *name=0 );
    QCheckBox( const QString &text, QWidget *parent, const char* name=0 );

qproperties:
    bool    isChecked() const;
    void    setChecked( bool check );
    
public:
    void    setNoChange();

    void    setTristate(bool y=TRUE);

    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;

protected:
    void    resizeEvent( QResizeEvent* );
    void    drawButton( QPainter * );
    void    drawButtonLabel( QPainter * );
    void    updateMask();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCheckBox( const QCheckBox & );
    QCheckBox &operator=( const QCheckBox & );
#endif
};


inline bool QCheckBox::isChecked() const
{ return isOn(); }

inline void QCheckBox::setChecked( bool check )
{ setOn( check ); }


#endif // QCHECKBOX_H
