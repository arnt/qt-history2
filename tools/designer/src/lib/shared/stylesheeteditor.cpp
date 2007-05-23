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
#include "csshighlighter_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormWindowCursorInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QExtensionManager>

#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QMessageBox>
#include "private/qcssparser_p.h"

static const char *styleSheetProperty = "styleSheet";

namespace qdesigner_internal {

StyleSheetEditor::StyleSheetEditor(QWidget *parent)
    : QTextEdit(parent)
{
    setTabStopWidth(fontMetrics().width(QLatin1Char(' '))*4);
}

// --- StyleSheetEditorDialog
StyleSheetEditorDialog::StyleSheetEditorDialog(QWidget *parent):
    QDialog(parent),
    m_buttonBox(new QDialogButtonBox),
    m_editor(new StyleSheetEditor),
    m_validityLabel(new QLabel(tr("Valid Style Sheet")))
{
    setWindowTitle(tr("Edit Style Sheet"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QGridLayout *layout = new QGridLayout;
    new CssHighlighter(m_editor->document());
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QObject::connect(m_editor, SIGNAL(textChanged()), this, SLOT(validateStyleSheet()));

    layout->addWidget(m_editor, 0, 0, 1, 2);;
    layout->addWidget(m_validityLabel, 1, 0, 1, 1);
    layout->addWidget(m_buttonBox, 1, 1, 1, 1);
    setLayout(layout);

    m_editor->setFocus();
    resize(430, 330);
}

QDialogButtonBox * StyleSheetEditorDialog::buttonBox() const
{
   return m_buttonBox;
}

QString StyleSheetEditorDialog::text() const
{
    return m_editor->toPlainText();
}

void StyleSheetEditorDialog::setText(const QString &t)
{
    m_editor->setText(t);
}

bool StyleSheetEditorDialog::isStyleSheetValid(const QString &styleSheet)
{
    QCss::Parser parser(styleSheet);
    QCss::StyleSheet sheet;
    if (parser.parse(&sheet))
        return true;
    QString fullSheet = QLatin1String("* { ");
    fullSheet += styleSheet;
    fullSheet += QLatin1Char('}');
    QCss::Parser parser2(fullSheet);
    return parser2.parse(&sheet);
}

void StyleSheetEditorDialog::validateStyleSheet()
{
    const QString text = m_editor->toPlainText();
    if (!isStyleSheetValid(text)) {
        m_validityLabel->setText(tr("Invalid Style Sheet"));
        m_validityLabel->setStyleSheet(QLatin1String("color: red"));
    } else {
        m_validityLabel->setText(tr("Valid Style Sheet"));
        m_validityLabel->setStyleSheet(QLatin1String("color: green"));
    }
}
// --- StyleSheetPropertyEditorDialog
StyleSheetPropertyEditorDialog::StyleSheetPropertyEditorDialog(QWidget *parent,
                                               QDesignerFormWindowInterface *fw,
                                               QWidget *widget):
    StyleSheetEditorDialog(parent),
    m_fw(fw),
    m_widget(widget)
{
    Q_ASSERT(m_fw != 0);

    QPushButton *apply = buttonBox()->addButton(QDialogButtonBox::Apply);
    QObject::connect(apply, SIGNAL(clicked()), this, SLOT(applyStyleSheet()));
    QObject::connect(buttonBox(), SIGNAL(accepted()), this, SLOT(applyStyleSheet()));

    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_fw->core()->extensionManager(), m_widget);
    Q_ASSERT(sheet != 0);
    setText(sheet->property(sheet->indexOf(QLatin1String(styleSheetProperty))).toString());
}

void StyleSheetPropertyEditorDialog::applyStyleSheet()
{
    m_fw->cursor()->setWidgetProperty(m_widget, QLatin1String(styleSheetProperty), QVariant(text()));
}

} // namespace qdesigner_internal
