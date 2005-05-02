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

#include <QtDesigner/extension.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QIcon>

class QWidget;
class QDesignerFormEditorInterface;

class QDesignerCustomWidgetInterface
{
public:
    virtual ~QDesignerCustomWidgetInterface() {}

    virtual QString name() const = 0;
    virtual QString group() const = 0;
    virtual QString toolTip() const = 0;
    virtual QString whatsThis() const = 0;
    virtual QString includeFile() const = 0;
    virtual QIcon icon() const = 0;

    virtual bool isContainer() const = 0;

    virtual QWidget *createWidget(QWidget *parent) = 0;

    virtual bool isInitialized() const { return false; }
    virtual void initialize(QDesignerFormEditorInterface *core) { Q_UNUSED(core); }

    virtual QString codeTemplate() const { return QString(); }
    virtual QString domXml() const { return QString(); }
};
Q_DECLARE_INTERFACE(QDesignerCustomWidgetInterface, "http://trolltech.com/Qt/Designer/CustomWidget")


class QDesignerCustomWidgetCollectionInterface
{
public:
    virtual ~QDesignerCustomWidgetCollectionInterface() {}

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const = 0;
};
Q_DECLARE_INTERFACE(QDesignerCustomWidgetCollectionInterface, "http://trolltech.com/Qt/Designer/CustomWidgetCollection")

#endif // CUSTOMWIDGET_H
