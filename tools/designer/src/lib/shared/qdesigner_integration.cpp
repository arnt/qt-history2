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

#include "qdesigner_integration.h"

// sdk
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowcursor.h>
#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/abstractpropertyeditor.h>
#include <QtDesigner/abstractwidgetbox.h>
#include <QtDesigner/abstractobjectinspector.h>

#include <QtCore/QVariant>

#include <QtCore/qdebug.h>

QDesignerIntegration::QDesignerIntegration(QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent),
      m_core(core)
{
    initialize();
}

QDesignerIntegration::~QDesignerIntegration()
{
}

void QDesignerIntegration::initialize()
{
    //
    // integrate the `Form Editor component'
    //
    connect(core()->propertyEditor(), SIGNAL(propertyChanged(QString,QVariant)),
            this, SLOT(updateProperty(QString,QVariant)));

    connect(core()->formWindowManager(), SIGNAL(formWindowAdded(QDesignerFormWindowInterface*)),
            this, SLOT(setupFormWindow(QDesignerFormWindowInterface*)));

    connect(core()->formWindowManager(), SIGNAL(activeFormWindowChanged(QDesignerFormWindowInterface*)),
            this, SLOT(updateActiveFormWindow(QDesignerFormWindowInterface*)));
}

void QDesignerIntegration::updateProperty(const QString &name, const QVariant &value)
{
    if (QDesignerFormWindowInterface *formWindow = core()->formWindowManager()->activeFormWindow()) {

        QDesignerFormWindowCursorInterface *cursor = formWindow->cursor();

        if (cursor->isWidgetSelected(formWindow->mainContainer())) {
            if (name == QLatin1String("windowTitle")) {
                QString filename = formWindow->fileName().isEmpty()
                        ? QString::fromUtf8("Untitled")
                        : formWindow->fileName();

                formWindow->setWindowTitle(QString::fromLatin1("%1 - (%2)")
                                           .arg(value.toString())
                                           .arg(filename));

            } else if (name == QLatin1String("geometry")) {
                QRect r = formWindow->topLevelWidget()->geometry();
                r.setSize(value.toRect().size());
                formWindow->topLevelWidget()->setGeometry(r);
                emit propertyChanged(formWindow, name, value);
                return;
            }
        }

        cursor->setProperty(name, value);

        if (name == QLatin1String("objectName") && core()->objectInspector()) {
            core()->objectInspector()->setFormWindow(formWindow);
        }

        emit propertyChanged(formWindow, name, value);
    }
}

void QDesignerIntegration::updateActiveFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_UNUSED(formWindow);
    updateSelection();
}

void QDesignerIntegration::setupFormWindow(QDesignerFormWindowInterface *formWindow)
{
    connect(formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
    connect(formWindow, SIGNAL(activated(QWidget*)), this, SLOT(activateWidget(QWidget*)));
    // ### connect(formWindow, SIGNAL(geometryChanged()), this, SLOT(updateGeometry()));
}

void QDesignerIntegration::updateGeometry()
{
    if (QDesignerFormWindowInterface *formWindow = core()->formWindowManager()->activeFormWindow()) {
        bool blocked = formWindow->blockSignals(true);
        formWindow->topLevelWidget()->resize(formWindow->mainContainer()->size());
        formWindow->blockSignals(blocked);
    }
}

void QDesignerIntegration::updateSelection()
{
    QDesignerFormWindowInterface *formWindow = core()->formWindowManager()->activeFormWindow();
    QWidget *selection = 0;

    if (formWindow)
        selection = formWindow->cursor()->selectedWidget(0);

    if (QDesignerObjectInspectorInterface *objectInspector = core()->objectInspector())
        objectInspector->setFormWindow(formWindow);

    if (QDesignerPropertyEditorInterface *propertyEditor = core()->propertyEditor()) {
        propertyEditor->setObject(selection);
        propertyEditor->setEnabled(formWindow && formWindow->cursor()->selectedWidgetCount() == 1);
    }
}

void QDesignerIntegration::activateWidget(QWidget *widget)
{
    // ### in-place editing here!!
    Q_UNUSED(widget);
}
