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

#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include "shared_global.h"
#include "abstractwidgetfactory.h"

#include <pluginmanager.h>

#include <QMap>
#include <QVariant>
#include <QPointer>

class QObject;
class QWidget;
class QLayout;
class AbstractFormEditor;
class ICustomWidget;

class QT_SHARED_EXPORT WidgetFactory: public AbstractWidgetFactory
{
public:
    WidgetFactory(AbstractFormEditor *core, QObject *parent = 0);
    ~WidgetFactory();

    virtual QWidget* containerOfWidget(QWidget *widget) const;
    virtual QWidget* widgetOfContainer(QWidget *widget) const;
    
    virtual QWidget *createWidget(const QString &className, QWidget *parentWidget) const;
    virtual QLayout *createLayout(QWidget *widget, QLayout *layout, int type) const;
    virtual void initialize(QObject *object) const;

    virtual AbstractFormEditor *core() const;

    static const char* classNameOf(QObject* o);
    
public slots:
    void loadPlugins();
    
private:
    AbstractFormEditor *m_core;
    QMap<QString, ICustomWidget*> m_customFactory;
};

#endif // WIDGETFACTORY_H
