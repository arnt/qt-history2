/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qhbox.h#11 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QHBOX_H
#define QHBOX_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#include "qframe.h"

class QBoxLayout;

class Q_EXPORT QHBox : public QFrame
{
    Q_OBJECT
public:
    QHBox( QWidget *parent=0, const char *name=0, WFlags f=0,  bool allowLines=TRUE  );


protected:
    QHBox( bool horizontal, QWidget *parent=0, const char *name=0, WFlags f=0,  bool allowLines=TRUE  );
    void frameChanged();
    void childEvent( QChildEvent * );

private:
    QBoxLayout *lay;
};

#endif //QHBOX_H
