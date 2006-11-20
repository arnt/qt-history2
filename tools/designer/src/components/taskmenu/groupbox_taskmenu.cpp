/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
TRANSLATOR qdesigner_internal::GroupBoxTaskMenu
*/

#include "groupbox_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/QtDesigner>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

GroupBoxTaskMenu::GroupBoxTaskMenu(QGroupBox *groupbox, QObject *parent)
    : QDesignerTaskMenu(groupbox, parent),
      m_groupbox(groupbox),
      m_editTitleAction(new QAction(tr("Change title..."), this))
   
{
    connect(m_editTitleAction, SIGNAL(triggered()), this, SLOT(editTitle()));
    m_taskActions.append(m_editTitleAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

GroupBoxTaskMenu::~GroupBoxTaskMenu()
{
}

QList<QAction*> GroupBoxTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void GroupBoxTaskMenu::editTitle()
{
    QDesignerFormWindowInterface *fw = formWindow();

    if (fw != 0) {
        connect(fw, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
        Q_ASSERT(m_groupbox->parentWidget() != 0);

        QStyleOption opt; // ## QStyleOptionGroupBox
        opt.init(m_groupbox);
        const QRect r = QRect(QPoint(), QSize(m_groupbox->width(),20));
        // ### m_groupbox->style()->subRect(QStyle::SR_GroupBoxTitle, &opt, m_groupbox);

        m_editor = new InPlaceEditor(m_groupbox, TextPropertyEditor::ValidationMultiLine, fw, m_groupbox->title(),r);

        connect(m_editor, SIGNAL(textChanged(QString)), this, SLOT(updateText(QString)));

    }
}

void GroupBoxTaskMenu::editIcon()
{
}

GroupBoxTaskMenuFactory::GroupBoxTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *GroupBoxTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QGroupBox *groupbox = qobject_cast<QGroupBox*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
            return new GroupBoxTaskMenu(groupbox, parent);
        }
    }

    return 0;
}

void GroupBoxTaskMenu::updateText(const QString &text)
{
    formWindow()->cursor()->setWidgetProperty(m_groupbox,
                                QLatin1String("title"), QVariant(text));
}

void GroupBoxTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

QAction *GroupBoxTaskMenu::preferredEditAction() const
{
    return m_editTitleAction;
}

