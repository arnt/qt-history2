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
#include "qdesigner_objectinspector.h"
#include "qdesigner_workbench.h"

#include <objectinspector/objectinspector.h>

#include <QtDesigner/abstractformeditor.h>

using namespace qdesigner_internal;

QDesignerObjectInspector::QDesignerObjectInspector(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    setObjectName(QLatin1String("ObjectInspector"));
    ObjectInspector *widget = new ObjectInspector(workbench->core(), this);
    workbench->core()->setObjectInspector(widget);

    setCentralWidget(widget);

    setWindowTitle(tr("Object Inspector"));
}

QDesignerObjectInspector::~QDesignerObjectInspector()
{
}

QRect QDesignerObjectInspector::geometryHint() const
{
    QRect g = workbench()->availableGeometry();
    int margin = workbench()->marginHint();

    QSize sz(g.width() * 1/4, g.height() * 1/6);

    return QRect((g.width() - sz.width() - margin), margin,
                  sz.width(), sz.height());
}
