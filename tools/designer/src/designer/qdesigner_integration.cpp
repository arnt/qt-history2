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

#include <QtCore/QFile>
#include <QtGui/QFileDialog>
#include <QtCore/QVariant>
#include <QtGui/QMessageBox>

#include <qdebug.h>

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
        cursor->setProperty(name, value);

        if (cursor->isWidgetSelected(formWindow->mainContainer())) {
            if (name == QLatin1String("windowTitle")) {
                QString filename = formWindow->fileName().isEmpty()
                        ? QString::fromUtf8("Untitled")
                    : formWindow->fileName();

                formWindow->setWindowTitle(QString::fromLatin1("%1 - (%2)")
                                           .arg(value.toString())
                                           .arg(filename));

            } else if (name == QLatin1String("geometry")) {
                formWindow->topLevelWidget()->setGeometry(value.toRect());
            }
        }

        if (name == QLatin1String("objectName") && core()->objectInspector()) {
            core()->objectInspector()->setFormWindow(formWindow);
        }

        emit propertyChanged(formWindow, name, value);
    }
}

void QDesignerIntegration::updateActiveFormWindow(AbstractFormWindow *formWindow)
{
    Q_UNUSED(formWindow);
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

    if (AbstractPropertyEditor *propertyEditor = core()->propertyEditor())
        propertyEditor->setObject(selection);
}

void QDesignerIntegration::activateWidget(QWidget *widget)
{
    // ### in-place editing here!!
    Q_UNUSED(widget);
}

bool QDesignerIntegration::saveForm(AbstractFormWindow *fw, const QString &saveFile)
{
    Q_ASSERT(fw && saveFile.isEmpty() == false);

    QFile f(saveFile);
    while (!f.open(QFile::WriteOnly)) {
        QMessageBox box(tr("Save Form?"),
                        tr("Could not open file: %1"
                                "\nReason: %2"
                                "\nWould you like to retry or change your file?")
                                .arg(f.fileName()).arg(f.errorString()),
                        QMessageBox::Warning,
                        QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                        QMessageBox::Cancel | QMessageBox::Escape, fw, Qt::WMacSheet);
        box.setButtonText(QMessageBox::Yes, tr("Retry"));
        box.setButtonText(QMessageBox::No, tr("Select New File"));
        switch(box.exec()) {
            case QMessageBox::Yes:
                break;
                case QMessageBox::No: {
                    QString fileName = QFileDialog::getSaveFileName(fw, tr("Save form as"),
                            QDir::home().absolutePath(), QString::fromLatin1("*.ui"));
                    if (fileName.isEmpty())
                        return false;
                    f.setFileName(fileName);
                    fw->setFileName(fileName);
                } break;
            case QMessageBox::Cancel:
                return false;
        }
    }
    QByteArray utf8Array = fw->contents().toUtf8();
    while (f.write(utf8Array, utf8Array.size()) != utf8Array.size()) {
        QMessageBox box(tr("Save Form?"),
                        tr("Could not write file: %1\nReason:%2\nWould you like to retry?")
                                .arg(f.fileName()).arg(f.errorString()),
                        QMessageBox::Warning,
                        QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, 0,
                        fw, Qt::WMacSheet);
        box.setButtonText(QMessageBox::Yes, tr("Retry"));
        box.setButtonText(QMessageBox::No, tr("Don't Retry"));
        switch(box.exec()) {
            case QMessageBox::Yes:
                f.resize(0);
                break;
            case QMessageBox::No:
                return false;
        }
    }

    fw->setDirty(false);

    return true;
}

