/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcheckbox.h#33 $
**
** Definition of QCheckBox class
**
** Created : 940222
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

#ifndef QCHECKBOX_H
#define QCHECKBOX_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H

#ifndef QT_NO_COMPLEXWIDGETS

class Q_EXPORT QCheckBox : public QButton
{
    Q_OBJECT
    Q_PROPERTY( bool checked READ isChecked WRITE setChecked )
    Q_PROPERTY( bool tristate READ isTristate WRITE setTristate )

public:
    QCheckBox( QWidget *parent, const char *name=0 );
    QCheckBox( const QString &text, QWidget *parent, const char* name=0 );

    bool    isChecked() const;
    void    setChecked( bool check );

    void    setNoChange();

    void    setTristate(bool y=TRUE);
    bool    isTristate() const;

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


#endif // QT_NO_COMPLEXWIDGETS

#endif // QCHECKBOX_H
