/**********************************************************************
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of Qt GUI Designer.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms.
**
**********************************************************************/

#include "mydialogimpl.h"
#include <qmessagebox.h>

MyDialogImpl::MyDialogImpl()
    : MyDialog()
{
}

void MyDialogImpl::upClicked()
{
    QMessageBox::information( this, tr( "Message" ), tr( "Item-Up clicked!" ) );
}
