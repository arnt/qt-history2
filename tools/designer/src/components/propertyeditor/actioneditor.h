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

#ifndef ACTIONEDITOR_H
#define ACTIONEDITOR_H

#include <QtGui/QDialog>

#include "ui_actioneditor.h"

namespace qdesigner { namespace components { namespace propertyeditor {

class ActionEditor: public QDialog
{
    Q_OBJECT
public:
    ActionEditor(QWidget *parent);
    virtual ~ActionEditor();

private:
    Ui::ActionEditor ui;
};

} } } // namespace qdesigner::components::propertyeditor

#endif // ACTIONEDITOR_H
