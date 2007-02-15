/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FORMWINDOWSETTINGS_H
#define FORMWINDOWSETTINGS_H

#include "ui_formwindowsettings.h"

class QDesignerFormWindowInterface;

namespace qdesigner_internal {
    class FormWindowBase;
}

class FormWindowSettings: public QDialog
{
    Q_OBJECT
public:
    FormWindowSettings(QDesignerFormWindowInterface *formWindow);

    virtual void accept();

private:
    Ui::FormWindowSettings ui;
    qdesigner_internal::FormWindowBase *m_formWindow;
};

#endif // FORMWINDOWSETTINGS_H

