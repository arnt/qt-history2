/****************************************************************************
**
** Definition of QRangeControlWidget class
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QRANGECONTROLWIDGET_H
#define QRANGECONTROLWIDGET_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other widgets.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qglobal.h"
#include "qframe.h"
#include "qrect.h"
#endif // QT_H

#ifndef QT_NO_RANGECONTROLWIDGET

class Q_EXPORT QRangeControlWidget : public QFrame
{
    Q_OBJECT
public:
    QRangeControlWidget( QWidget* parent = 0, const char* name = 0 );
    ~QRangeControlWidget();

    QRect upRect() const;
    QRect downRect() const;

    void setUpEnabled( bool on );
    void setDownEnabled( bool on );

    enum ButtonSymbols { UpDownArrows, PlusMinus };
    virtual void	setButtonSymbols( ButtonSymbols bs );
    ButtonSymbols	buttonSymbols() const;

signals:
    void stepUpPressed();
    void stepDownPressed();

public slots:
    void setEnabled( bool on );

protected:
    void mousePressEvent( QMouseEvent *e );
    void resizeEvent( QResizeEvent* ev );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void wheelEvent( QWheelEvent * );
    void drawContents( QPainter *p );
    void styleChange( QStyle& );

private slots:
    void stepUp();
    void stepDown();

private:
    class Private;
    Private * d;

    void arrange();
    void updateDisplay();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QRangeControlWidget( const QRangeControlWidget& );
    QRangeControlWidget& operator=( const QRangeControlWidget& );
#endif
};

#endif // QT_NO_RANGECONTROLWIDGET

#endif // QRANGECONTROL_H
