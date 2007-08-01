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
TRANSLATOR qdesigner_internal::ScriptDialog
*/

#include "scriptdialog_p.h"
#include "qscripthighlighter_p.h"

#include <abstractdialoggui_p.h>

#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QMessageBox>
#include <QtScript/QScriptEngine>

namespace qdesigner_internal {

    // ScriptDialog
    ScriptDialog::ScriptDialog(QDesignerDialogGuiInterface *m_dialogGui, QWidget *parent) :
        QDialog(parent),
        m_dialogGui(m_dialogGui),
        m_textEdit(new QTextEdit)
    {
        setWindowTitle(tr("Edit script"));
        setModal(true);

        QVBoxLayout *vboxLayout = new QVBoxLayout(this);

        const QString textHelp = tr("\
<html>Enter a Qt Script snippet to be executed while loading the form.<br>\
The widget and its children are accessible via the \
variables <i>widget</i> and <i>childWidgets</i>, respectively.");
        m_textEdit->setToolTip(textHelp);
        m_textEdit->setWhatsThis(textHelp);
        m_textEdit->setMinimumSize(QSize(600, 400));
        vboxLayout->addWidget(m_textEdit);
        new QScriptHighlighter(m_textEdit->document());
        // button box
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        connect(buttonBox , SIGNAL(rejected()), this, SLOT(reject()));
        connect(buttonBox , SIGNAL(accepted()), this, SLOT(slotAccept()));
        vboxLayout->addWidget(buttonBox);
    }

    bool ScriptDialog::editScript(QString &script)
    {
        m_textEdit->setText(script);
        if (exec() != Accepted)
            return false;

        script = trimmedScript();
        return true;
    }

    void ScriptDialog::slotAccept()
    {
        if (checkScript())
            accept();
    }

    QString ScriptDialog::trimmedScript() const
    {
        // Ensure a single newline
        QString rc = m_textEdit->toPlainText().trimmed();
        if (!rc.isEmpty())
            rc += QLatin1Char('\n');
        return rc;
    }

    bool ScriptDialog::checkScript()
    {
        const QString script = trimmedScript();
        if (script.isEmpty())
            return true;
        QScriptEngine scriptEngine;
        if (scriptEngine.canEvaluate(script))
            return true;
        m_dialogGui->message(this, QDesignerDialogGuiInterface::ScriptDialogMessage, QMessageBox::Warning,
                             windowTitle(), tr("Syntax error"), QMessageBox::Ok);
        return  false;
    }
} // namespace qdesigner_internal
