#include <QtGui/QRegExpValidator>
#include <QtGui/QMessageBox>

#include <abstractwidgetdatabase.h>
#include "promotetocustomwidgetdialog.h"

PromoteToCustomWidgetDialog::PromoteToCustomWidgetDialog(AbstractWidgetDataBase *db,
                                                        const QString &base_class_name,
                                                        QWidget *parent)
    : QDialog(parent)
{
    m_db = db;
    setupUi(this);
    m_class_name_input->setValidator(new QRegExpValidator(QRegExp("[_a-zA-Z][_a-zA-Z0-9]*"), m_class_name_input));
    m_base_class_name_label->setText(base_class_name);
    connect(m_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_class_name_input, SIGNAL(textChanged(const QString&)), this, SLOT(checkInputs()));
    connect(m_header_file_input, SIGNAL(textChanged(const QString&)), this, SLOT(checkInputs()));
    m_ok_button->setEnabled(false);
    m_automatic_include = true;
}

void PromoteToCustomWidgetDialog::checkInputs()
{
    if (sender() == m_class_name_input) {
        if (m_automatic_include) {
            QString class_name = customClassName();
            m_header_file_input->blockSignals(true);
            if (class_name.isEmpty())
                m_header_file_input->clear();
            else
                m_header_file_input->setText(class_name.toLower() + QLatin1String(".h"));
            m_header_file_input->blockSignals(false);
        }
    } else if (sender() == m_header_file_input) {
        m_automatic_include = false;
    }

    m_ok_button->setEnabled(!customClassName().isEmpty()
                                && !includeFile().isEmpty());
}

void PromoteToCustomWidgetDialog::accept()
{
    QString custom_class_name = customClassName();
    QString include_file = includeFile();

    AbstractWidgetDataBaseItem *item = m_db->item(m_db->indexOfClassName(custom_class_name));
    if (item != 0 && !item->isPromoted()) {
        QMessageBox::warning(0, tr("Incorrect class name"),
                                tr("<b>%1</b> cannot be used as the class of the promoted"
                                    " widget, as a class of that name already exists"
                                    " and is not a promoted widget.").arg(custom_class_name),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    QDialog::accept();
}

QString PromoteToCustomWidgetDialog::includeFile() const
{
    return m_header_file_input->text();
}

QString PromoteToCustomWidgetDialog::customClassName() const
{
    return m_class_name_input->text();
}

