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
#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>
#include <abstractformwindowmanager.h>
#include <abstractpropertyeditor.h>
#include <abstractwidgetbox.h>
#include <abstractobjectinspector.h>

#include <QtCore/QVariant>

#include <QtCore/qdebug.h>

QDesignerIntegration::QDesignerIntegration(AbstractFormEditor *core, QObject *parent)
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
    connect(core()->propertyEditor(), SIGNAL(propertyChanged(const QString&, const QVariant& )),
            this, SLOT(updateProperty(const QString&, const QVariant& )));

    connect(core()->formWindowManager(), SIGNAL(formWindowAdded(AbstractFormWindow* )),
            this, SLOT(setupFormWindow(AbstractFormWindow* )));

    connect(core()->formWindowManager(), SIGNAL(activeFormWindowChanged(AbstractFormWindow* )),
            this, SLOT(updateActiveFormWindow(AbstractFormWindow* )));
}

void QDesignerIntegration::updateProperty(const QString &name, const QVariant &value)
{
    if (AbstractFormWindow *formWindow = core()->formWindowManager()->activeFormWindow()) {

        AbstractFormWindowCursor *cursor = formWindow->cursor();

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

void QDesignerIntegration::updateActiveFormWindow(AbstractFormWindow *formWindow)
{
    Q_UNUSED(formWindow);
    updateSelection();
}

void QDesignerIntegration::setupFormWindow(AbstractFormWindow *formWindow)
{
    connect(formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
    connect(formWindow, SIGNAL(activated(QWidget *)), this, SLOT(activateWidget(QWidget *)));
    // ### connect(formWindow, SIGNAL(geometryChanged()), this, SLOT(updateGeometry()));
}

void QDesignerIntegration::updateGeometry()
{
    if (AbstractFormWindow *formWindow = core()->formWindowManager()->activeFormWindow()) {
        bool blocked = formWindow->blockSignals(true);
        formWindow->topLevelWidget()->resize(formWindow->mainContainer()->size());
        formWindow->blockSignals(blocked);
    }
}

void QDesignerIntegration::updateSelection()
{
    AbstractFormWindow *formWindow = core()->formWindowManager()->activeFormWindow();
    QWidget *selection = 0;

    if (formWindow)
        selection = formWindow->cursor()->selectedWidget(0);

    if (AbstractObjectInspector *objectInspector = core()->objectInspector())
        objectInspector->setFormWindow(formWindow);

    if (AbstractPropertyEditor *propertyEditor = core()->propertyEditor()) {
        propertyEditor->setObject(selection);
        propertyEditor->setEnabled(formWindow && formWindow->cursor()->selectedWidgetCount() == 1);
    }
}

void QDesignerIntegration::activateWidget(QWidget *widget)
{
    // ### in-place editing here!!
    Q_UNUSED(widget);
}
