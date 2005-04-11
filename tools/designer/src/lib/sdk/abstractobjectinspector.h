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

#ifndef ABSTRACTOBJECTINSPECTOR_H
#define ABSTRACTOBJECTINSPECTOR_H

#include <QtDesigner/sdk_global.h>

#include <QtGui/QWidget>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QT_SDK_EXPORT QDesignerObjectInspectorInterface: public QWidget
{
    Q_OBJECT
public:
    QDesignerObjectInspectorInterface(QWidget *parent, Qt::WindowFlags flags = 0);
    virtual ~QDesignerObjectInspectorInterface();

    virtual QDesignerFormEditorInterface *core() const;

public slots:
    virtual void setFormWindow(QDesignerFormWindowInterface *formWindow) = 0;
};

#endif // ABSTRACTOBJECTINSPECTOR_H
