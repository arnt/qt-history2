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

#ifndef ABSTRACTACTIONEDITOR_H
#define ABSTRACTACTIONEDITOR_H

#include <QtDesigner/sdk_global.h>

#include <QtGui/QWidget>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QDESIGNER_SDK_EXPORT QDesignerActionEditorInterface: public QWidget
{
    Q_OBJECT
public:
    QDesignerActionEditorInterface(QWidget *parent, Qt::WindowFlags flags = 0);
    virtual ~QDesignerActionEditorInterface();

    virtual QDesignerFormEditorInterface *core() const;

public Q_SLOTS:
    virtual void setFormWindow(QDesignerFormWindowInterface *formWindow) = 0;
};

#endif // ABSTRACTACTIONEDITOR_H
