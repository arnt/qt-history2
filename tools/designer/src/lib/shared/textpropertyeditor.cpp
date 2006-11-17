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

#include "textpropertyeditor_p.h"
#include "propertylineedit_p.h"

#include <QtGui/QLineEdit>
#include <QtGui/QRegExpValidator>
#include <QtGui/QResizeEvent>
#include <qdebug.h>

namespace {
    const QChar NewLineChar('\n');
    const QLatin1String EscapedNewLine("\\n");

    // A validator that replaces newline characters by literal "\\n".
    // While it is not possible to actually type a newline  characters,
    // it can be pasted into the line edit.
    class NewLineValidator : public QValidator {
    public:
        NewLineValidator ( QObject * parent ) : QValidator(parent ) {}
        virtual void fixup ( QString & input ) const;
        virtual State validate ( QString & input, int &pos) const;
    };

    void NewLineValidator::fixup ( QString & input ) const {
        input.replace(NewLineChar,EscapedNewLine);
    }

    QValidator::State NewLineValidator::validate ( QString & input, int &/* pos */) const {
        fixup (input);
        return Acceptable;
    }
}

namespace qdesigner_internal {
    // TextPropertyEditor
    TextPropertyEditor::TextPropertyEditor(EmbeddingMode embeddingMode,
                                           ValidationMode validationMode,
                                           QWidget *parent) :
        QWidget(parent),
        m_ValidationMode(validationMode),
        m_lineEdit(new PropertyLineEdit(this, validationMode == ValidationNone))
    {
        switch ( embeddingMode) {
        case EmbeddingNone:
            break;
        case EmbeddingTreeView:
            m_lineEdit->setFrame(false);
            break;
        case EmbeddingInPlace:
            m_lineEdit->setFrame(false);
            Q_ASSERT(parent);
            m_lineEdit->setBackgroundRole(parent->backgroundRole());
            break;
        }

        switch (m_ValidationMode) {
        case ValidationNone:
            // convert newlines pasted in to the LineEdit.
            m_lineEdit->setValidator(new  NewLineValidator(m_lineEdit));
            break;
        case ValidationObjectName:
            setRegExpValidator("[_a-zA-Z][_a-zA-Z0-9]{,1023}");
             break;
        case ValidationObjectNameScope:
            setRegExpValidator("[_a-zA-Z:][_a-zA-Z0-9:]{,1023}");
            break;
        }

        setFocusProxy(m_lineEdit);

        connect(m_lineEdit,SIGNAL(editingFinished()),this,SIGNAL(editingFinished()));
        connect(m_lineEdit,SIGNAL(textChanged(QString)),this,SLOT(slotTextChanged(QString)));
    }

    void TextPropertyEditor::setRegExpValidator(const QString &pattern)
    {
        const QRegExp regExp(pattern);
        Q_ASSERT(regExp.isValid());
        m_lineEdit->setValidator(new QRegExpValidator(regExp,m_lineEdit));
    }

    QString TextPropertyEditor::text() const
    {
        return m_cachedText;
    }

    void TextPropertyEditor::setText(const QString &text)
    {
        m_cachedText = text;
        m_lineEdit->setText(stringToEditorString(text, m_ValidationMode));
    }

    void  TextPropertyEditor::slotTextChanged(const QString &text) {
        m_cachedText = editorStringToString(text, m_ValidationMode);
        emit textChanged(m_cachedText);
    }

    void TextPropertyEditor::selectAll() {
        m_lineEdit->selectAll();
    }

    void TextPropertyEditor::setAlignment(Qt::Alignment alignment) {
        m_lineEdit->setAlignment(alignment);
    }

    void TextPropertyEditor::resizeEvent ( QResizeEvent * event ) {
        m_lineEdit->resize( event->size());
    }

    QSize TextPropertyEditor::sizeHint () const {
        return  m_lineEdit->sizeHint ();
    }

    // Replace newline characters literal "\n"  for inline editing in mode ValidationNone
    QString TextPropertyEditor::stringToEditorString(const QString &s, ValidationMode  validationMode) {
        if (s.isEmpty() || validationMode != ValidationNone)
            return s;

        QString rc(s);
        // protect backslashes
        rc.replace(QLatin1String("\\"), QLatin1String("\\\\"));
        // escape newlines
        rc.replace(NewLineChar, EscapedNewLine);
        return rc;

    }

    // Replace literal "\n"  by actual new lines for inline editing in mode ValidationNone
    // Note: As the properties are updated while the user types, it is important
    // that trailing slashes ('bla\') are not deleted nor ignored, else this will
    // cause jumping of the  cursor
    QString  TextPropertyEditor::editorStringToString(const QString &s, ValidationMode  validationMode) {
        if (s.isEmpty() || validationMode != ValidationNone)
            return s;

        QString rc(s);
        for (int pos = 0; (pos = rc.indexOf('\\',pos)) >= 0 ; ) {
            // found an escaped character. If not a newline or at end of string, leave as is, else insert '\n'
            const int nextpos = pos + 1;
            if (nextpos  >= rc.length())  // trailing '\\'
                 break;
            // Escaped NewLine
            if (rc.at(nextpos) ==  QChar('n'))
                 rc[nextpos] =  NewLineChar;
            // Remove escape, go past escaped
            rc.remove(pos,1);
            pos++;
        }
        return rc;
    }
}

