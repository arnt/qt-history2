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
TRANSLATOR qdesigner_internal::StyleSheetEditorDialog
*/

#include "stylesheeteditor_p.h"
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtDesigner/QtDesigner>
#include <QtGui/QMessageBox>
#include "private/qcssparser_p.h"

namespace qdesigner_internal {

StyleSheetEditor::StyleSheetEditor(QWidget *parent)
    : QTextEdit(parent)
{
}

StyleSheetEditorDialog::StyleSheetEditorDialog(QWidget *fw, QWidget *widget)
    : QDialog(fw), m_widget(widget)
{
    m_fw = qobject_cast<QDesignerFormWindowInterface *>(fw);
    Q_ASSERT(m_fw != 0);
    setWindowTitle(tr("Edit Style Sheet"));
    QGridLayout *layout = new QGridLayout;
    m_editor = new StyleSheetEditor;
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QPushButton *apply = buttonBox->button(QDialogButtonBox::Apply);
    QObject::connect((const QObject *)apply, SIGNAL(clicked()), this, SLOT(applyStyleSheet()));
    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(applyStyleSheet()));

    validityLabel = new QLabel(tr("Valid Style Sheet"));
    QObject::connect(m_editor, SIGNAL(textChanged()), this, SLOT(validateStyleSheet()));

    layout->addWidget(m_editor, 0, 0, 1, 2);;
    layout->addWidget(validityLabel, 1, 0, 1, 1);
    layout->addWidget(buttonBox, 1, 1, 1, 1);
    setLayout(layout);

    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_fw->core()->extensionManager(), m_widget);
    Q_ASSERT(sheet != 0);
    m_editor->setText(sheet->property(sheet->indexOf("styleSheet")).toString());

    m_editor->setFocus();
    resize(430, 330);
}

StyleSheetEditor *StyleSheetEditorDialog::editor() const
{
    return m_editor;
}

void StyleSheetEditorDialog::applyStyleSheet()
{
    QString text = m_editor->toPlainText();
    m_fw->cursor()->setWidgetProperty(m_widget, "styleSheet", QVariant(text));
}

bool StyleSheetEditorDialog::isStyleSheetValid(const QString &styleSheet)
{
    QCss::Parser parser(styleSheet);
    QCss::StyleSheet sheet;
    if (parser.parse(&sheet))
        return true;
    QCss::Parser parser2(QLatin1String("* { ") + styleSheet + QLatin1String("}"));
    return parser2.parse(&sheet);
}

void StyleSheetEditorDialog::validateStyleSheet()
{
    QString text = m_editor->toPlainText();
    if (!isStyleSheetValid(text)) {
        validityLabel->setText(tr("Invalid Style Sheet"));
        validityLabel->setStyleSheet("color: red");
    } else {
        validityLabel->setText(tr("Valid Style Sheet"));
        validityLabel->setStyleSheet("color: green");
    }
}

} // namespace qdesigner_internal
