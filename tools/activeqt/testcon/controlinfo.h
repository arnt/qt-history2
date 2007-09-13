/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CONTROLINFO_H
#define CONTROLINFO_H

#include <QtCore/qglobal.h>
#include "ui_controlinfo.h"

QT_BEGIN_NAMESPACE

class ControlInfo : public QDialog, Ui::ControlInfo
{
    Q_OBJECT
public:
    ControlInfo(QWidget *parent);

    void setControl(QWidget *activex);
};

QT_END_NAMESPACE

#endif // CONTROLINFO_H
