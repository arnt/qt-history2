/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qwhatsthis.h#4 $
**
** Definition of QWhatsThis class
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QWHATSTHIS_H
#define QWHATSTHIS_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

class QToolButton;
class QPopupMenu;

// it's a class! it's a struct! it's a namespace! IT'S WHATS THIS?!
class QWhatsThis
{
public:
    static void add( QWidget *, const char *, bool deepCopy = TRUE );
    static void add( QWidget *, const QPixmap &,
		     const char *, const char *, bool deepCopy = TRUE );
    static void remove( QWidget * );
    static const char * textFor( QWidget * );

    static QToolButton * whatsThisButton( QWidget * parent );
    //static void enterWhatsThisMode();

    //static void say( const char *, QWidget * near );

    //static int addMenuEntry( QPopupMenu *, QWidget *, const char * = 0 );
};

#endif
