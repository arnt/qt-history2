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

#include "label_taskmenu.h"
#include "inplace_editor.h"

#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>
#include <abstractformwindowmanager.h>
#include <richtexteditor.h>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

LabelTaskMenu::LabelTaskMenu(QLabel *label, QObject *parent)
    : QDesignerTaskMenu(label, parent),
      m_label(label)
{
    m_editTextAction= new QAction(this);
    m_editTextAction->setText(tr("Edit Label Text"));
    connect(m_editTextAction, SIGNAL(triggered()), this, SLOT(editText()));
    m_taskActions.append(m_editTextAction);
}

LabelTaskMenu::~LabelTaskMenu()
{
}

QAction *LabelTaskMenu::preferredEditAction() const
{
    return m_editTextAction;
}

QList<QAction*> LabelTaskMenu::taskActions() const
{
    return QDesignerTaskMenu::taskActions() + m_taskActions;
}

void LabelTaskMenu::editText()
{
    m_formWindow = AbstractFormWindow::findFormWindow(m_label);
    if (!m_formWindow.isNull()) {
        RichTextEditor *editor = new RichTextEditor(m_formWindow);
        connect(m_formWindow, SIGNAL(selectionChanged()), editor, SLOT(deleteLater()));

        Q_ASSERT(m_label->parentWidget() != 0);
        editor->setObjectName(QLatin1String("__qt__passive_m_editor"));

        editor->setText(m_label->text());
        editor->setFormat(m_label->textFormat());
        editor->setDefaultFont(m_label->font());
        editor->selectAll();
        connect(editor, SIGNAL(textChanged(const QString &)), this,
                    SLOT(updateText(const QString&)));

        QStyleOption opt;
        opt.init(m_label);
        QRect r = opt.rect;

        editor->setGeometry(QRect(m_label->mapTo(m_label->window(), r.topLeft()), r.size()));
        editor->setFocus();
        editor->show();
    }
}

void LabelTaskMenu::editIcon()
{
    qDebug() << "edit icon";
}

LabelTaskMenuFactory::LabelTaskMenuFactory(QExtensionManager *extensionManager)
    : DefaultExtensionFactory(extensionManager)
{
}

QObject *LabelTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QLabel *label = qobject_cast<QLabel*>(object)) {
        if (iid == Q_TYPEID(ITaskMenu)) {
            return new LabelTaskMenu(label, parent);
        }
    }

    return 0;
}

void LabelTaskMenu::updateText(const QString &text)
{
    m_formWindow->cursor()->setWidgetProperty(m_label, QLatin1String("text"), QVariant(text));
}

