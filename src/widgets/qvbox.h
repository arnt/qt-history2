/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvbox.h#9 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QVBOX_H
#define QVBOX_H

#ifndef QT_H
#include "qhbox.h"
#endif // QT_H

class Q_EXPORT QVBox : public QHBox
{
    Q_OBJECT
public:
    QVBox( QWidget *parent=0, const char *name=0, WFlags f=0,  bool allowLines=TRUE );
};

#endif //QVBOX_H
