/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvbox.h#4 $
**
** Definition of vbox layout widget
**
** Created : 980220
**
** Copyright (C) 1996-1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QVBOX_H
#define QVBOX_H

#ifndef QT_H
#include "qhbox.h"
#endif // QT_H

class QVBox : public QHBox
{
    Q_OBJECT
public:
    QVBox( QWidget *parent=0, const char *name=0, WFlags f=0 );
};

#endif //QVBOX_H
