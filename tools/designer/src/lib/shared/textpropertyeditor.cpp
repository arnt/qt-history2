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
#include "stylesheeteditor_p.h"

#include <QtGui/QLineEdit>
#include <QtGui/QRegExpValidator>
#include <QtGui/QResizeEvent>
#include <qdebug.h>

namespace {
    const QChar NewLineChar(QLatin1Char('\n'));
    const QLatin1String EscapedNewLine("\\n");

    // A validator that replaces offending strings
    class ReplacementValidator : public QValidator {
    public:
        ReplacementValidator (QObject * parent,
                              const QString &offending,
                              const QString &replacement);
        virtual void fixup ( QString & input ) const;
        virtual State validate ( QString & input, int &pos) const;
    private:
        const QString m_offending;
        const QString m_replacement;
    };

    ReplacementValidator::ReplacementValidator (QObject * parent,
                                        const QString &offending,
                                        const QString &replacement) :
      QValidator(parent ),
      m_offending(offending),
      m_replacement(replacement)
    {
    }

    void ReplacementValidator::fixup ( QString & input ) const {
        input.replace(m_offending, m_replacement);
    }

    QValidator::State ReplacementValidator::validate ( QString & input, int &/* pos */) const {
        fixup (input);
        return Acceptable;
    }

    // A validator for style sheets. Does newline handling and validates sheets.
    class StyleSheetValidator : public ReplacementValidator {
    public:
        StyleSheetValidator (QObject * parent);
        virtual State validate ( QString & input, int &pos) const;
    };
    StyleSheetValidator::StyleSheetValidator (QObject * parent) :
       ReplacementValidator(parent, NewLineChar, EscapedNewLine)
    {
    }

    QValidator::State StyleSheetValidator::validate ( QString & input, int &pos) const
    {
        // base class
        const State state = ReplacementValidator:: validate(input, pos);
        if (state != Acceptable)
            return state;
        // now check style sheet, create string with newlines
        const QString styleSheet = qdesigner_internal::TextPropertyEditor::editorStringToString(input, qdesigner_internal::ValidationStyleSheet);
        const bool valid = qdesigner_internal::StyleSheetEditorDialog::isStyleSheetValid(styleSheet);
        return valid ? Acceptable : Intermediate;
    }

}

namespace qdesigner_internal {
    // TextPropertyEditor
    TextPropertyEditor::TextPropertyEditor(QWidget *parent,
                                           EmbeddingMode embeddingMode,
                                           TextPropertyValidationMode validationMode) :
        QWidget(parent),
        m_validationMode(ValidationSingleLine),
        m_lineEdit(new PropertyLineEdit(this))
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

        setFocusProxy(m_lineEdit);

        connect(m_lineEdit,SIGNAL(editingFinished()),this,SIGNAL(editingFinished()));
        connect(m_lineEdit,SIGNAL(textChanged(QString)),this,SLOT(slotTextChanged(QString)));

        setTextPropertyValidationMode(validationMode);
    }

    void TextPropertyEditor::setTextPropertyValidationMode(TextPropertyValidationMode vm) {
        m_validationMode = vm;
        m_lineEdit->setWantNewLine(m_validationMode == ValidationMultiLine);
        switch (m_validationMode) {
        case ValidationStyleSheet:
            m_lineEdit->setValidator(new  StyleSheetValidator(m_lineEdit));
            break;
        case ValidationMultiLine:
            // Set a  validator that replaces newline characters by literal "\\n".
            // While it is not possible to actually type a newline  characters,
            // it can be pasted into the line edit.
            m_lineEdit->setValidator(new  ReplacementValidator(m_lineEdit, NewLineChar, EscapedNewLine));
            break;
        case ValidationSingleLine:
            // Set a  validator that replaces newline characters by a blank.
            m_lineEdit->setValidator(new  ReplacementValidator(m_lineEdit, NewLineChar, QString(QLatin1Char(' '))));
            break;
        case ValidationObjectName:
            setRegExpValidator(QLatin1String("[_a-zA-Z][_a-zA-Z0-9]{,1023}"));
             break;
        case ValidationObjectNameScope:
            setRegExpValidator(QLatin1String("[_a-zA-Z:][_a-zA-Z0-9:]{,1023}"));
            break;
        }

        setFocusProxy(m_lineEdit);

        disconnect(m_lineEdit,SIGNAL(editingFinished()),this,SIGNAL(editingFinished()));
        disconnect(m_lineEdit,SIGNAL(textChanged(QString)),this,SLOT(slotTextChanged(QString)));
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
        m_lineEdit->setText(stringToEditorString(text, m_validationMode));
    }

    void  TextPropertyEditor::slotTextChanged(const QString &text) {
        m_cachedText = editorStringToString(text, m_validationMode);
        emit textChanged(m_cachedText);
    }

    void TextPropertyEditor::selectAll() {
        m_lineEdit->selectAll();
    }

    void TextPropertyEditor::clear() {
        m_lineEdit->clear();
    }

    void TextPropertyEditor::setAlignment(Qt::Alignment alignment) {
        m_lineEdit->setAlignment(alignment);
    }

    void TextPropertyEditor::installEventFilter(QObject *filterObject)
    {
        if (m_lineEdit)
            m_lineEdit->installEventFilter(filterObject);
    }

    void TextPropertyEditor::resizeEvent ( QResizeEvent * event ) {
        m_lineEdit->resize( event->size());
    }

    QSize TextPropertyEditor::sizeHint () const {
        return  m_lineEdit->sizeHint ();
    }

    QSize TextPropertyEditor::minimumSizeHint () const {
        return  m_lineEdit->minimumSizeHint ();
    }

    // Returns whether newline characters are valid in validationMode.
    bool TextPropertyEditor::multiLine(TextPropertyValidationMode validationMode) {
        return validationMode == ValidationMultiLine || validationMode == ValidationStyleSheet;
    }

    // Replace newline characters literal "\n"  for inline editing in mode ValidationMultiLine
    QString TextPropertyEditor::stringToEditorString(const QString &s, TextPropertyValidationMode  validationMode) {
        if (s.isEmpty() || !multiLine(validationMode))
            return s;

        QString rc(s);
        // protect backslashes
        rc.replace(QLatin1String("\\"), QLatin1String("\\\\"));
        // escape newlines
        rc.replace(NewLineChar, EscapedNewLine);
        return rc;

    }

    // Replace literal "\n"  by actual new lines for inline editing in mode ValidationMultiLine
    // Note: As the properties are updated while the user types, it is important
    // that trailing slashes ('bla\') are not deleted nor ignored, else this will
    // cause jumping of the  cursor
    QString  TextPropertyEditor::editorStringToString(const QString &s, TextPropertyValidationMode  validationMode) {
        if (s.isEmpty() || !multiLine(validationMode))
            return s;

        QString rc(s);
        for (int pos = 0; (pos = rc.indexOf(QLatin1Char('\\'),pos)) >= 0 ; ) {
            // found an escaped character. If not a newline or at end of string, leave as is, else insert '\n'
            const int nextpos = pos + 1;
            if (nextpos  >= rc.length())  // trailing '\\'
                 break;
            // Escaped NewLine
            if (rc.at(nextpos) ==  QChar(QLatin1Char('n')))
                 rc[nextpos] =  NewLineChar;
            // Remove escape, go past escaped
            rc.remove(pos,1);
            pos++;
        }
        return rc;
    }
}

