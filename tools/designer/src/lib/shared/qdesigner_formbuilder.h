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

#ifndef QDESIGNER_FORMBUILDER_H
#define QDESIGNER_FORMBUILDER_H

#include "shared_global.h"

#include <formbuilder.h>
#include <QtCore/QMap>

class AbstractFormEditor;
class ICustomWidget;

class QT_SHARED_EXPORT QDesignerFormBuilder: public FormBuilder
{
public:
    QDesignerFormBuilder(AbstractFormEditor *core);

    QWidget *createWidgetFromContents(const QString &contents, QWidget *parentWidget = 0);

    virtual QWidget *createWidget(DomWidget *ui_widget, QWidget *parentWidget = 0)
    { return FormBuilder::create(ui_widget, parentWidget); }

    inline AbstractFormEditor *core() const
    { return m_core; }

protected:
    virtual QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name);
    virtual bool addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget);
    virtual bool addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout);

private:
    AbstractFormEditor *m_core;
    QMap<QString, ICustomWidget*> m_customFactory;
};

#endif // QDESIGNER_FORMBUILDER_H
