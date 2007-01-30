#ifndef INTERFACEPAGE_H
#define INTERFACEPAGE_H

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

#include <QFrame>
#include "ui_interfacepage.h"
class InterfacePage : public QFrame, public Ui_InterfacePage
{
    Q_OBJECT
public:
    InterfacePage(QWidget *parent = 0);
    void load();
    void save();
signals:
    void changed();
};

#endif
