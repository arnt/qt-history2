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

#ifndef BUDDYEDITOR_TOOL_H
#define BUDDYEDITOR_TOOL_H

#include "buddyeditor_global.h"

#include <abstractformwindowtool.h>

class AbstractFormEditor;
class AbstractFormWindow;

class QT_BUDDYEDITOR_EXPORT BuddyEditorTool: public AbstractFormWindowTool
{
    Q_OBJECT
public:
    BuddyEditorTool(AbstractFormWindow *formWindow, QObject *parent = 0);
    virtual ~BuddyEditorTool();

    virtual AbstractFormEditor *core() const;
    virtual AbstractFormWindow *formWindow() const;

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

private:
    AbstractFormWindow *m_formWindow;
};

#endif // BUDDYEDITOR_TOOL_H
