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

#ifndef SAVEFORMASTEMPLATE_H
#define SAVEFORMASTEMPLATE_H

#include "ui_saveformastemplate.h"

class SaveFormAsTemplate: public QDialog
{
    Q_OBJECT
public:
    SaveFormAsTemplate(QWidget *parent = 0);
    virtual ~SaveFormAsTemplate();

private:
    Ui::SaveFormAsTemplate ui;
};

#endif // SAVEFORMASTEMPLATE_H
