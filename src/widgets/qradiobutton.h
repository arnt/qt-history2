/****************************************************************************
**
** Definition of QRadioButton class.
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

#ifndef QRADIOBUTTON_H
#define QRADIOBUTTON_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H

#ifndef QT_NO_RADIOBUTTON

class Q_GUI_EXPORT QRadioButton : public QButton
{
    Q_OBJECT
    Q_PROPERTY( bool checked READ isChecked WRITE setChecked )
    Q_OVERRIDE( bool autoMask DESIGNABLE true SCRIPTABLE true )

public:
    QRadioButton( QWidget *parent=0, const char* name=0 );
    QRadioButton( const QString &text, QWidget *parent=0, const char* name=0 );

    bool    isChecked() const;

    QSize    sizeHint() const;

public slots:
    virtual void    setChecked( bool check );

protected:
    bool    hitButton( const QPoint & ) const;
    void    drawButton( QPainter * );
    void    drawButtonLabel( QPainter * );
    void    updateMask();

    void    resizeEvent( QResizeEvent* );

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

#endif // QT_NO_RADIOBUTTON

#endif // QRADIOBUTTON_H
