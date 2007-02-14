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
TRANSLATOR qdesigner_internal::ScriptErrorDialog
*/

#include "scripterrordialog_p.h"

#include <QtGui/QTextEdit>
#include <QtGui/QTextCursor>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPen>

static void formatError(const QFormScriptRunner::Error &error,
                        QTextCursor &cursor)
{
    const QTextCharFormat oldFormat = cursor.charFormat();
    // Message
    cursor.insertText(QObject::tr("An error occurred while running the scripts for \"%1\":\n").arg(error.objectName));

    QTextCharFormat format(oldFormat);

    // verbatim listing
    format.setFontFamily(QLatin1String("Courier"));
    cursor.insertText(error.script, format);

    const QString newLine(QLatin1Char('\n'));

    cursor.insertText(newLine);

    // red error
    format = oldFormat;
    format.setTextOutline(QPen(Qt::red));
    cursor.insertText(error.errorMessage, format);
    cursor.insertText(newLine);
    cursor.setCharFormat (oldFormat);
}

namespace qdesigner_internal {

    // ScriptErrorDialog
    ScriptErrorDialog::ScriptErrorDialog(const Errors& errors, QWidget *parent) :
        QDialog(parent),
        m_textEdit(new QTextEdit)
    {
        setWindowTitle(tr("Script errors"));
        setModal(true);

        QVBoxLayout *vboxLayout = new QVBoxLayout(this);

        m_textEdit->setReadOnly(true);
        m_textEdit->setMinimumSize(QSize(600, 400));
        vboxLayout->addWidget(m_textEdit);
        // button box
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(buttonBox , SIGNAL(rejected()), this, SLOT(reject()));
        vboxLayout->addWidget(buttonBox);

        // Generate text
        QTextCursor cursor = m_textEdit->textCursor();
        cursor.movePosition (QTextCursor::End);
        foreach (const QFormScriptRunner::Error error, errors)
            formatError(error, cursor);
    }
} // namespace qdesigner_internal
