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

#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <extension.h>
#include <QObject>
#include <QString>
#include <QIcon>

class QWidget;
class AbstractFormEditor;

struct ICustomWidget
{
    virtual ~ICustomWidget() {}

    virtual QString name() const = 0;
    virtual QString group() const = 0;
    virtual QString toolTip() const = 0;
    virtual QString whatsThis() const = 0;
    virtual QString includeFile() const = 0;
    virtual QIcon icon() const = 0;

    virtual bool isContainer() const = 0;
    virtual bool isForm() const = 0;

    virtual QWidget *createWidget(QWidget *parent) = 0;

//
// IDesignerCustomWidget
//
    virtual bool isInitialized() const
    { return false; }

    virtual void initialize(AbstractFormEditor *core)
    { Q_UNUSED(core); }

    virtual QString codeTemplate() const
    { return QString::null; }
};

Q_DECLARE_EXTENSION_INTERFACE(ICustomWidget, "http://trolltech.com/Qt/IDE/CustomWidget")

#endif // CUSTOMWIDGET_H
