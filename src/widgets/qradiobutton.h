/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qradiobutton.h#43 $
**
** Definition of QRadioButton class
**
** Created : 940222
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

#ifndef QRADIOBUTTON_H
#define QRADIOBUTTON_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H

#if QT_FEATURE_WIDGETS

class Q_EXPORT QRadioButton : public QButton
{
    Q_OBJECT
    Q_PROPERTY( bool checked READ isChecked WRITE setChecked )

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
    void    focusInEvent( QFocusEvent * );

    void    updateMask();

private:
    void    init();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QRadioButton( const QRadioButton & );
    QRadioButton &operator=( const QRadioButton & );
#endif
};


inline bool QRadioButton::isChecked() const
{ return isOn(); }

#endif // QT_FEATURE_WIDGETS

#endif // QRADIOBUTTON_H
