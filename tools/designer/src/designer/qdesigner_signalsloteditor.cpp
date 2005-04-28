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

#include "qdesigner.h"
#include "qdesigner_signalsloteditor.h"
#include "qdesigner_workbench.h"
#include "qdesigner_settings.h"

#include <signalsloteditor/signalsloteditorwindow.h>

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindowmanager.h>

#include <QtCore/qdebug.h>


QDesignerSignalSlotEditor::QDesignerSignalSlotEditor(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    using namespace qdesigner_internal;

    setObjectName(QLatin1String("SignalSlotEditorWindow"));
    SignalSlotEditorWindow *widget
        = new SignalSlotEditorWindow(workbench->core(), this);

    setCentralWidget(widget);

    setWindowTitle(tr("Signal/slot editor"));
}

QDesignerSignalSlotEditor::~QDesignerSignalSlotEditor()
{
}

QRect QDesignerSignalSlotEditor::geometryHint() const
{
    QRect g = workbench()->availableGeometry();
    int margin = workbench()->marginHint();

    QSize sz(g.width() * 1/3, g.height() * 1/6);
    QRect r(QPoint(0, 0), sz);
    r.moveCenter(g.center());
    r.moveTop(margin);

    return r;
}
