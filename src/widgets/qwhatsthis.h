/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qwhatsthis.h#22 $
**
** Definition of QWhatsThis class
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

#ifndef QWHATSTHIS_H
#define QWHATSTHIS_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

#if QT_FEATURE_WIDGETS

#include "qcursor.h"

class QToolButton;
class QPopupMenu;
class QStyleSheet;

class Q_EXPORT QWhatsThis: public Qt
{
public:
    QWhatsThis( QWidget *);
    virtual ~QWhatsThis();

    virtual QString text( const QPoint & );

    // the common static functions
    static void add( QWidget *, const QString &);
    static void remove( QWidget * );
    static QString textFor( QWidget *, const QPoint & pos = QPoint() );

    static QToolButton * whatsThisButton( QWidget * parent );

    static void enterWhatsThisMode();
    static bool inWhatsThisMode();
    static void leaveWhatsThisMode( const QString& = QString::null, const QPoint& pos = QCursor::pos() );

};

#endif // QT_FEATURE_WIDGETS

#endif // QWHATSTHIS_H
