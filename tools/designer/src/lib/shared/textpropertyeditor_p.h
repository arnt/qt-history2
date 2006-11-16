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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef TEXTPROPERTYEDITOR_H
#define TEXTPROPERTYEDITOR_H

#include "shared_global_p.h"

#include <QtGui/QWidget>


namespace qdesigner_internal {

    class PropertyLineEdit;

    // Editor for text properties. Does escaping of newline characters
    // and provides validation modes. The interface corresponds to
    // that of QLineEdit.
    class QDESIGNER_SHARED_EXPORT TextPropertyEditor : public QWidget
    {
        TextPropertyEditor(const TextPropertyEditor &);
        TextPropertyEditor& operator=(const TextPropertyEditor &);
        Q_OBJECT
    public:
        enum EmbeddingMode {
            // Stand-alone widget
            EmbeddingNone,
                // Disable frame
                EmbeddingTreeView,
                // For editing in forms
                EmbeddingInPlace
        };

        enum ValidationMode {
            ValidationNone,
                // Allow only for identifier characters
                ValidationObjectName,
                // Allow only for identifier characters and colons
                ValidationObjectNameScope };

        TextPropertyEditor(EmbeddingMode embeddingMode = EmbeddingNone, ValidationMode validationMode = ValidationNone, QWidget *parent = 0);

        QString text() const;

        virtual QSize sizeHint () const;

        void setAlignment(Qt::Alignment alignment);

        // Replace newline characters by literal "\n" for inline editing
        // in mode ValidationNone
        static QString stringToEditorString(const QString &s, ValidationMode validationMode = ValidationNone);

        // Replace literal "\n"  by actual new lines in mode ValidationNone
        static QString editorStringToString(const QString &s, ValidationMode validationMode = ValidationNone);

    signals:
        void textChanged(const QString &text);
        void editingFinished ();

    public slots:
        void setText(const QString &text);
        void selectAll();

    protected:
        void resizeEvent ( QResizeEvent * event );

    private slots:
        void slotTextChanged(const QString &text);
    private:
        void setRegExpValidator(const QString &pattern);

        const ValidationMode m_ValidationMode;
        PropertyLineEdit* m_lineEdit;

        // Cached text containing real newline characters.
        QString m_cachedText;
    };
}

#endif // TEXTPROPERTYEDITOR_H
