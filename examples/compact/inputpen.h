/****************************************************************************
** $Id: //depot/qt/main/examples/compact/inputpen.h#4 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qimpen/qimpeninput.h"

class QWSServer;

class QWSPenInput : public QIMPenInput
{
    Q_OBJECT
public:
    QWSPenInput( QWidget *parent, const char *name, int WFlags );

protected slots:
    void keyPress( unsigned int unicode );
};

