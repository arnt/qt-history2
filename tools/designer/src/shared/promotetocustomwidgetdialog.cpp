#include <QtGui/QRegExpValidator>
#include <QtGui/QMessageBox>

#include <abstractwidgetdatabase.h>
#include "promotetocustomwidgetdialog.h"

#include <qdebug.h>

PromoteToCustomWidgetDialog::PromoteToCustomWidgetDialog(AbstractWidgetDataBase *db,
                                                        const QString &base_class_name,
                                                        QWidget *parent)
    : QDialog(parent)
{
    m_db = db;
    
    setupUi(this);
    
    m_class_name_input->addItem(QString());
    for (int i = 0; i < db->count(); ++i) {
        AbstractWidgetDataBaseItem *item = db->item(i);
        if (!item->isPromoted())
            continue;
        m_promoted_list.append(qMakePair(item->name(), item->includeFile()));
        m_class_name_input->addItem(item->name());
    }

    m_class_name_input->setValidator(new QRegExpValidator(QRegExp("[_a-zA-Z][_a-zA-Z0-9]*"), m_class_name_input));
    m_base_class_name_label->setText(base_class_name);
    connect(m_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_class_name_input->lineEdit(), SIGNAL(textChanged(const QString&)),
            this, SLOT(checkInputs()));
    connect(m_class_name_input, SIGNAL(activated(const QString&)),
            this, SLOT(setIncludeForClass(const QString&)));
    connect(m_header_file_input, SIGNAL(textChanged(const QString&)), this, SLOT(checkInputs()));
    m_ok_button->setEnabled(false);
    m_automatic_include = true;

    m_class_name_input->setFocus();
}

void PromoteToCustomWidgetDialog::checkInputs()
{
    m_header_file_input->blockSignals(true);
    if (sender() == m_class_name_input->lineEdit()) {
        if (m_automatic_include) {
            QString class_name = customClassName();
            if (class_name.isEmpty())
                m_header_file_input->clear();
            else
                m_header_file_input->setText(class_name.toLower() + QLatin1String(".h"));
        }
    } else if (sender() == m_header_file_input) {
        m_automatic_include = false;
    }
    m_header_file_input->blockSignals(false);

    m_ok_button->setEnabled(!customClassName().isEmpty()
                                && !includeFile().isEmpty());
}

void PromoteToCustomWidgetDialog::setIncludeForClass(const QString &name)
{
    m_header_file_input->blockSignals(true);
    m_header_file_input->clear();
    foreach (PromotedWidgetInfo info, m_promoted_list) {
        if (info.first == name) {
            m_header_file_input->setText(info.second);
            break;
        }
    }
    m_header_file_input->blockSignals(false);
    m_automatic_include = true;
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
    return m_class_name_input->lineEdit()->text();
}

