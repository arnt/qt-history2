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
TRANSLATOR qdesigner_internal::LabelTaskMenu
*/

#include "label_taskmenu.h"
#include "inplace_editor.h"
#include <QtDesigner/QtDesigner>
#include <richtexteditor_p.h>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

LabelTaskMenu::LabelTaskMenu(QLabel *label, QObject *parent)
    : QDesignerTaskMenu(label, parent),
      m_label(label),
      m_editRichTextAction(new QAction(tr("Change rich text..."), this)),
      m_editPlainTextAction(new QAction(tr("Change plain text..."), this))
{
    connect(m_editPlainTextAction, SIGNAL(triggered()), this, SLOT(editPlainText()));
    m_taskActions.append(m_editPlainTextAction);

    connect(m_editRichTextAction, SIGNAL(triggered()), this, SLOT(editRichText()));
    m_taskActions.append(m_editRichTextAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

LabelTaskMenu::~LabelTaskMenu()
{
}

QAction *LabelTaskMenu::preferredEditAction() const
{
    if (m_label->textFormat () == Qt::PlainText) return m_editPlainTextAction;
    return Qt::mightBeRichText(m_label->text()) ? m_editRichTextAction : m_editPlainTextAction;
}

QList<QAction*> LabelTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void LabelTaskMenu::editRichText()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_label);
    if (!m_formWindow.isNull()) {
        RichTextEditorDialog dlg(m_formWindow);
        Q_ASSERT(m_label->parentWidget() != 0);
        RichTextEditor *editor = dlg.editor();

        editor->setDefaultFont(m_label->font());
        editor->setText(m_label->text());
        editor->selectAll();
        editor->setFocus();

        if (dlg.exec()) {
            const QString text = editor->text(m_label->textFormat());
            m_formWindow->cursor()->setProperty(QLatin1String("text"), QVariant(text));
        }
    }
}

void LabelTaskMenu::editPlainText()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_label);
    if (!m_formWindow.isNull()) {
        connect(m_formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
        Q_ASSERT(m_label->parentWidget() != 0);
        
        QStyleOptionButton opt;
        opt.init(m_label);
        
        m_editor = new InPlaceEditor(m_label, ValidationMultiLine, m_formWindow,m_label->text(),opt.rect);

        connect(m_editor, SIGNAL(textChanged(QString)), this, SLOT(updateText(QString)));   
    }
}

void LabelTaskMenu::editIcon()
{
}

LabelTaskMenuFactory::LabelTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *LabelTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QLabel *label = qobject_cast<QLabel*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
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

