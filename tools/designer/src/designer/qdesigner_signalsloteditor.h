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

#ifndef QDESIGNER_SIGNALSLOTEDITOR_H
#define QDESIGNER_SIGNALSLOTEDITOR_H

#include "qdesigner_toolwindow.h"

class QDesignerWorkbench;

class QDesignerSignalSlotEditor: public QDesignerToolWindow
{
    Q_OBJECT
public:
    QDesignerSignalSlotEditor(QDesignerWorkbench *workbench);
    virtual ~QDesignerSignalSlotEditor();

    virtual QRect geometryHint() const;
};

#endif // QDESIGNER_SIGNALSLOTEDITOR_H
