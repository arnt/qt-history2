/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qwhatsthis.h#17 $
**
** Definition of QWhatsThis class
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

#ifndef QWHATSTHIS_H
#define QWHATSTHIS_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

class QToolButton;
class QPopupMenu;
class QMLStyleSheet;

class Q_EXPORT QWhatsThis: public Qt
{
public:
    QWhatsThis( QWidget *);
    virtual ~QWhatsThis();

    virtual QString text( const QPoint & );

    // the common static functions
    static void add( QWidget *, const QString &);
    static void remove( QWidget * );
    static QString textFor( QWidget *, const QPoint & pos = QPoint(0,0) );

    static QToolButton * whatsThisButton( QWidget * parent );
    
    static QMLStyleSheet* styleSheet();
    static void setStyleSheet( QMLStyleSheet* styleSheet );

    static void enterWhatsThisMode();

};

#endif
