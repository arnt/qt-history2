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
TRANSLATOR qdesigner_internal::PromoteToCustomWidgetDialog
*/

#include "promotetocustomwidgetdialog_p.h"
#include "ui_promotetocustomwidgetdialog.h"

#include <QtDesigner/QtDesigner>

#include <QtGui/QRegExpValidator>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include <QtCore/qdebug.h>

namespace {
    // Format header for HTML rich text
    QString formatHTMLHeader(const QString &h, bool global) {
        QString rc;
        if (global) {
            rc += QLatin1String("&lt;");
        } else {
            rc += QLatin1Char('"');
        }
        rc += h;
        if (global) {
            rc += QLatin1String("&gt;");
        } else {
            rc += QLatin1Char('"');
        }
        return rc;
    }
}

namespace qdesigner_internal {

PromoteToCustomWidgetDialog::PromoteToCustomWidgetDialog(QDesignerWidgetDataBaseInterface *db,
                                                         const WidgetDataBaseItemList &candidates,
                                                         const QString &base_class_name,
                                                         QWidget *parent)  : 
    QDialog(parent),
    m_base_class_name(base_class_name),
    m_ui(new Ui::PromoteToCustomWidgetDialog),
    m_db(db)
{
    setModal(true);
    m_ui->setupUi(this);

    m_ui->m_class_name_input->addItem(QString());
    // Create a map with includes for checking
    const WidgetDataBaseItemList::const_iterator cend = candidates.constEnd();
    for (WidgetDataBaseItemList::const_iterator it = candidates.constBegin(); it != cend; ++it) {
        const QString className = (*it)->name();
        const bool global = (*it)->includeType() == QDesignerWidgetDataBaseItemInterface::IncludeGlobal;
        m_promotedHash.insert(className ,  PromotedWidgetInfo((*it)->includeFile(),  global));
        m_ui->m_class_name_input->addItem(className);
    }

    m_ui->m_class_name_input->setValidator(new QRegExpValidator(QRegExp(QLatin1String("[_a-zA-Z:][:_a-zA-Z0-9]*")), m_ui->m_class_name_input));
    m_ui->m_base_class_name_label->setText(base_class_name);

    connect(m_ui->m_class_name_input->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
    connect(m_ui->m_header_file_input, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));

    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    m_ui->m_class_name_input->setFocus();
}

PromoteToCustomWidgetDialog::~PromoteToCustomWidgetDialog()
{
    delete m_ui;
}

void PromoteToCustomWidgetDialog::checkInputs()
{
    if (sender() == m_ui->m_class_name_input->lineEdit()) {
        setIncludeForClass(customClassName());

    }

    const bool enabled = !customClassName().isEmpty() && !includeFile().isEmpty();
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(enabled);
    m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(!enabled);
}

void PromoteToCustomWidgetDialog::setIncludeForClass(const QString &name)
{
    // Figure out an include file for the class name entered. 
    // Check if a matching one exists in the candidate list.
    const bool blocked = m_ui->m_header_file_input->blockSignals(true);

    do {
        
        if (name.isEmpty()) {
            m_ui->m_header_file_input->clear();
            m_ui->m_globalCheckBox->setChecked(false);
            break;
        }
    
        const PromotedWidgetInfoHash::const_iterator pit = m_promotedHash.constFind(name);
        if (pit != m_promotedHash.constEnd()) {
            m_ui->m_header_file_input->setText(pit.value().first);
            m_ui->m_globalCheckBox->setChecked(pit.value().second);
            break;
        }

        // Suggest a name
        QString suggestedHeader = name.toLower().replace(QLatin1String("::"), QLatin1String("_"));
        suggestedHeader += QLatin1String(".h");
        m_ui->m_header_file_input->setText(suggestedHeader);
        m_ui->m_globalCheckBox->setChecked(false);
              
    } while (false);
        
    m_ui->m_header_file_input->blockSignals(blocked);
}

void PromoteToCustomWidgetDialog::warn(const QString &caption, const QString &what)
{
    QMessageBox::warning(this, caption, what, QMessageBox::Ok, QMessageBox::NoButton);
}
    
    
bool PromoteToCustomWidgetDialog::ask(const QString &caption, const QString &what)
{
    return QMessageBox::warning(this, caption, what,  QMessageBox::Yes, QMessageBox::No) !=  QMessageBox::No;
}

void PromoteToCustomWidgetDialog::accept()
{
    const QString custom_class_name = customClassName();
    const QString include_file = includeFile();
    const bool globalInclude = isGlobalInclude();

    const QDesignerWidgetDataBaseItemInterface *item = m_db->item(m_db->indexOfClassName(custom_class_name));
    if (item != 0) {
        if (!item->isPromoted()) {
            warn(tr("Conflicting class name"),
                 tr("<b>%1</b> cannot be used as the class of the promoted"
                    " widget, as a class of that name already exists"
                    " and is not a promoted widget.").arg(custom_class_name));
            return;
        }
        if (item->extends() != m_base_class_name) {
            warn(tr("Conflicting class name"),
                 tr("<b>%1</b> cannot be used as the class of the promoted"
                    " widget, as a class of that name already exists"
                    " and extends <b>%2</b>.").arg(custom_class_name).arg(item->extends()));
            return;
        }
    }

    // existing class with different include?
    const PromotedWidgetInfoHash::const_iterator pit = m_promotedHash.constFind( custom_class_name);
    if (pit != m_promotedHash.constEnd()) {
        // mismatch?
        if (pit.value().first != include_file || pit.value().second != globalInclude) {
            const bool overWriteInclude = 
            ask(tr("Conflicting include file"),
                tr("<b>%1</b> has been previously specified as the"
                   " include file for <b>%2</b>. Do you want to"
                   " change all instances to use <b>%3</b> instead?")
                .arg(formatHTMLHeader(pit.value().first, pit.value().second))
                .arg(custom_class_name)
                .arg(formatHTMLHeader(include_file, globalInclude)));
        if (!overWriteInclude)
            return;
        }
    }

    QDialog::accept();
}

QString PromoteToCustomWidgetDialog::includeFile() const
{
    return m_ui->m_header_file_input->text();
}

QString PromoteToCustomWidgetDialog::customClassName() const
{
    return m_ui->m_class_name_input->lineEdit()->text();
}
    
bool  PromoteToCustomWidgetDialog::isGlobalInclude() const
{
    return m_ui->m_globalCheckBox->isChecked();
}

} // namespace qdesigner_internal
