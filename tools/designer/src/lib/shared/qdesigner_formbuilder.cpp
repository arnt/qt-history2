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

#include "qdesigner_formbuilder.h"
#include "qdesigner_widget.h"

#include <container.h>
#include <customwidget.h>
#include <pluginmanager.h>
#include <qextensionmanager.h>
#include <abstractformeditor.h>
#include <abstracticoncache.h>
#include <resourcefile.h>

#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QWidget>

QDesignerFormBuilder::QDesignerFormBuilder(AbstractFormEditor *core)
    : m_core(core)
{
    Q_ASSERT(m_core);

    PluginManager *pluginManager = m_core->pluginManager();

    m_customFactory.clear();
    QStringList plugins = pluginManager->registeredPlugins();

    foreach (QString plugin, plugins) {
        QObject *o = pluginManager->instance(plugin);

        if (ICustomWidget *c = qobject_cast<ICustomWidget*>(o)) {
            if (!c->isInitialized())
                c->initialize(m_core);

            m_customFactory.insert(c->name(), c);
        }
    }
}

QWidget *QDesignerFormBuilder::createWidgetFromContents(const QString &contents, QWidget *parentWidget)
{
    QByteArray data = contents.toUtf8();
    QBuffer buffer(&data);
    return load(&buffer, parentWidget);
}


QWidget *QDesignerFormBuilder::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    if (ICustomWidget *c = m_customFactory.value(widgetName)) {
        QWidget *widget = c->createWidget(parentWidget);
        widget->setObjectName(name);
        return widget;
    }

    if (widgetName == QLatin1String("QLabel")) {
        QLabel *label = new QDesignerLabel(parentWidget);
        label->setObjectName(name);
        return label;
    }

    QWidget *widget = FormBuilder::createWidget(widgetName, parentWidget, name);
    if (!widget) {
        // ### qWarning("failed to create a widget for type %s", widgetName.toLatin1().constData());
        widget = new QWidget(parentWidget);
        widget->setObjectName(name);
    }

    return widget;
}

bool QDesignerFormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    if (FormBuilder::addItem(ui_widget, widget, parentWidget))
        return true;

    if (IContainer *container = qt_extension<IContainer*>(m_core->extensionManager(), parentWidget)) {
        container->addWidget(widget);
        return true;
    }

    return false;
}

bool QDesignerFormBuilder::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    return FormBuilder::addItem(ui_item, item, layout);
}

QIcon QDesignerFormBuilder::nameToIcon(const QString &filePath, const QString &qrcPath)
{
    QString icon_path = filePath;
    QString qrc_path = QFileInfo(QDir(workingDirectory()), qrcPath).absoluteFilePath();

    if (!qrc_path.isEmpty()) {
        ResourceFile rf(qrc_path);
        if (rf.load())
            return QIcon(rf.resolvePath(filePath));
    } else {
        icon_path = QFileInfo(QDir(workingDirectory()), filePath).absoluteFilePath();
        return QIcon(icon_path);
    }

    return QIcon();
}

