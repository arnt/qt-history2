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

#include "textedit_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowcursor.h>
#include <QtDesigner/abstractformwindowmanager.h>
#include <richtexteditor.h>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

TextEditTaskMenu::TextEditTaskMenu(QTextEdit *textEdit, QObject *parent)
    : QDesignerTaskMenu(textEdit, parent),
      m_textEdit(textEdit)
{
    m_editTextAction= new QAction(this);
    m_editTextAction->setText(tr("Edit Html"));
    connect(m_editTextAction, SIGNAL(triggered()), this, SLOT(editText()));
    m_taskActions.append(m_editTextAction);
}

TextEditTaskMenu::~TextEditTaskMenu()
{
}

QAction *TextEditTaskMenu::preferredEditAction() const
{
    return m_editTextAction;
}

QList<QAction*> TextEditTaskMenu::taskActions() const
{
    return QDesignerTaskMenu::taskActions() + m_taskActions;
}

void TextEditTaskMenu::editText()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_textEdit);
    if (!m_formWindow.isNull()) {
        RichTextEditorDialog *dlg = new RichTextEditorDialog(m_formWindow);
        Q_ASSERT(m_textEdit->parentWidget() != 0);
        RichTextEditor *editor = dlg->editor();

        editor->setDefaultFont(m_textEdit->font());
        editor->setText(m_textEdit->toHtml());
        editor->selectAll();

        if (dlg->exec()) {
            QString text = editor->text(Qt::RichText);
            m_formWindow->cursor()->setWidgetProperty(m_textEdit, QLatin1String("html"), QVariant(text));
        }

        delete dlg;
    }
}

void TextEditTaskMenu::editIcon()
{
    qDebug() << "edit icon";
}

TextEditTaskMenuFactory::TextEditTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *TextEditTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QTextEdit *textEdit = qobject_cast<QTextEdit*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
            return new TextEditTaskMenu(textEdit, parent);
        }
    }

    return 0;
}

void TextEditTaskMenu::updateText(const QString &text)
{
    m_formWindow->cursor()->setWidgetProperty(m_textEdit, QLatin1String("html"), QVariant(text));
}

