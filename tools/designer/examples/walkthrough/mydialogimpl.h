/**********************************************************************
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of Qt GUI Designer.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms.
**
**********************************************************************/

#include "mydialog.h"

class MyDialogImpl : public MyDialog
{
public:
    MyDialogImpl();
    
public slots:
    void upClicked();
    
};
