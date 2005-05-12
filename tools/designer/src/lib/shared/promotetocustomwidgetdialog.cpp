/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/QRegExpValidator>
#include <QtGui/QMessageBox>

#include <QtDesigner/abstractwidgetdatabase.h>
#include "promotetocustomwidgetdialog.h"

#include <QtCore/qdebug.h>

PromoteToCustomWidgetDialog::PromoteToCustomWidgetDialog(QDesignerWidgetDataBaseInterface *db,
                                                        const QString &base_class_name,
                                                        QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    m_db = db;
    m_base_class_name = base_class_name;

    setupUi(this);

    m_class_name_input->addItem(QString());
    for (int i = 0; i < db->count(); ++i) {
        QDesignerWidgetDataBaseItemInterface *item = db->item(i);
        if (!item->isPromoted())
            continue;
        if (item->extends() != base_class_name)
            continue;
        m_promoted_list.append(qMakePair(item->name(), item->includeFile()));
        m_class_name_input->addItem(item->name());
    }

    m_class_name_input->setValidator(new QRegExpValidator(QRegExp(QLatin1String("[_a-zA-Z:][:_a-zA-Z0-9]*")), m_class_name_input));
    m_base_class_name_label->setText(base_class_name);
    connect(m_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_class_name_input->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(checkInputs()));
    connect(m_class_name_input, SIGNAL(activated(QString)),
            this, SLOT(setIncludeForClass(QString)));
    connect(m_header_file_input, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));
    m_ok_button->setEnabled(false);
    m_automatic_include = true;

    m_class_name_input->setFocus();
}

void PromoteToCustomWidgetDialog::checkInputs()
{
    bool blocked = m_header_file_input->blockSignals(true);
    if (sender() == m_class_name_input->lineEdit()) {
        if (m_automatic_include) {
            QString class_name = customClassName();
            if (class_name.isEmpty())
                m_header_file_input->clear();
            else
                m_header_file_input->setText(class_name.toLower().replace(QLatin1String("::"), QLatin1String("_")) + QLatin1String(".h"));
        }
    } else if (sender() == m_header_file_input) {
        m_automatic_include = false;
    }
    m_header_file_input->blockSignals(blocked);

    m_ok_button->setEnabled(!customClassName().isEmpty()
                                && !includeFile().isEmpty());
}

void PromoteToCustomWidgetDialog::setIncludeForClass(const QString &name)
{
    bool blocked = m_header_file_input->blockSignals(true);
    m_header_file_input->clear();
    foreach (PromotedWidgetInfo info, m_promoted_list) {
        if (info.first == name) {
            m_header_file_input->setText(info.second);
            break;
        }
    }
    m_header_file_input->blockSignals(blocked);
    m_automatic_include = true;
}

void PromoteToCustomWidgetDialog::accept()
{
    QString custom_class_name = customClassName();
    QString include_file = includeFile();

    QDesignerWidgetDataBaseItemInterface *item = m_db->item(m_db->indexOfClassName(custom_class_name));
    if (item != 0) {
        if (!item->isPromoted()) {
            QMessageBox::warning(0, tr("Conflicting class name"),
                                    tr("<b>%1</b> cannot be used as the class of the promoted"
                                        " widget, as a class of that name already exists"
                                        " and is not a promoted widget.")
                                            .arg(custom_class_name),
                                    QMessageBox::Ok, QMessageBox::NoButton);
            return;
        }
        if (item->extends() != m_base_class_name) {
            QMessageBox::warning(0, tr("Conflicting class name"),
                                    tr("<b>%1</b> cannot be used as the class of the promoted"
                                        " widget, as a class of that name already exists"
                                        " and extends <b>%2</b>.")
                                            .arg(custom_class_name)
                                            .arg(item->extends()),
                                    QMessageBox::Ok, QMessageBox::NoButton);
            return;
        }
    }

    foreach (PromotedWidgetInfo info, m_promoted_list) {
        if (info.first == custom_class_name) {
            if (info.second != include_file) {
                int result
                    = QMessageBox::warning(0, tr("Conflicting include file"),
                            tr("<b>%1</b> has been previously specified as the"
                            " include file for <b>%2</b>. Do you want to"
                            " change all instances to use <b>%3</b> instead?")
                                    .arg(info.second)
                                    .arg(custom_class_name)
                                    .arg(include_file),
                            QMessageBox::Yes, QMessageBox::No);
                if (result == QMessageBox::No)
                    return;
            }
            break;
        }
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

