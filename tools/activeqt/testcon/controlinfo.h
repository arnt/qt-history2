/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CONTROLINFO_H
#define CONTROLINFO_H

#include "ui_controlinfo.h"

class ControlInfo : public QDialog, Ui::ControlInfo
{
    Q_OBJECT
public:
    ControlInfo(QWidget *parent);

    void setControl(QWidget *activex);
};

#endif // CONTROLINFO_H
