/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MYDIALOG_H
#define MYDIALOG_H

#include "ui_mydialog.h"

class MyDialog : public QDialog, public Ui::MyDialog
{
    Q_OBJECT

public:
    MyDialog(QWidget *parent = 0);
};

#endif
