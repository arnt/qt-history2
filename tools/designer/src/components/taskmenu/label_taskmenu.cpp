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
    QAction *action = 0;

    action = new QAction(this);
    action->setText(tr("Edit Label Text"));
    connect(action, SIGNAL(triggered()), this, SLOT(editText()));
    m_taskActions.append(action);
}

LabelTaskMenu::~LabelTaskMenu()
{
}

QList<QAction*> LabelTaskMenu::taskActions() const
{
    return QDesignerTaskMenu::taskActions() + m_taskActions;
}

void LabelTaskMenu::editText()
{
    m_formWindow = AbstractFormWindow::findFormWindow(m_label);
    if (!m_formWindow.isNull()) {
        connect(m_formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
        Q_ASSERT(m_label->parentWidget() != 0);

        m_editor = new InPlaceEditor(m_label);
        m_editor->setObjectName("__qt__passive_m_editor");

        m_editor->setFrame(false);
        m_editor->setText(m_label->text());
        m_editor->selectAll();
        m_editor->setBackgroundRole(m_label->backgroundRole());
        connect(m_editor, SIGNAL(returnPressed()), m_editor, SLOT(deleteLater()));
        connect(m_editor, SIGNAL(textChanged(const QString &)), this, SLOT(updateText(const QString&)));

        QStyleOption opt;
        opt.init(m_label);
        QRect r = opt.rect;

        m_editor->setAttribute(Qt::WA_NoChildEventsForParent);
        m_editor->setGeometry(QRect(m_label->mapTo(m_label->window(), r.topLeft()), r.size()));
        m_editor->setFocus();
        m_editor->show();
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
    m_formWindow->cursor()->setProperty(QLatin1String("text"), QVariant(text));
}

void LabelTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

