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

#include "newdynamicpropertydialog.h"
#include "ui_newdynamicpropertydialog.h"
#include <abstractdialoggui_p.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

NewDynamicPropertyDialog::NewDynamicPropertyDialog(QDesignerDialogGuiInterface *dialogGui,
                                                       QWidget *parent)   :
    QDialog(parent),
    m_dialogGui(dialogGui),
    m_ui(new Ui::NewDynamicPropertyDialog)
{
    m_ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_ui->m_comboBox->addItem(QLatin1String("String"),      QVariant(QVariant::String));
    m_ui->m_comboBox->addItem(QLatin1String("StringList"),  QVariant(QVariant::StringList));
    m_ui->m_comboBox->addItem(QLatin1String("Char"),        QVariant(QVariant::Char));
    m_ui->m_comboBox->addItem(QLatin1String("ByteArray"),   QVariant(QVariant::ByteArray));
    m_ui->m_comboBox->addItem(QLatin1String("Url"),         QVariant(QVariant::Url));
    m_ui->m_comboBox->addItem(QLatin1String("Bool"),        QVariant(QVariant::Bool));
    m_ui->m_comboBox->addItem(QLatin1String("Int"),         QVariant(QVariant::Int));
    m_ui->m_comboBox->addItem(QLatin1String("UInt"),        QVariant(QVariant::UInt));
    m_ui->m_comboBox->addItem(QLatin1String("LongLong"),    QVariant(QVariant::LongLong));
    m_ui->m_comboBox->addItem(QLatin1String("ULongLong"),   QVariant(QVariant::ULongLong));
    m_ui->m_comboBox->addItem(QLatin1String("Double"),      QVariant(QVariant::Double));
    m_ui->m_comboBox->addItem(QLatin1String("Size"),        QVariant(QVariant::Size));
    m_ui->m_comboBox->addItem(QLatin1String("SizeF"),       QVariant(QVariant::SizeF));
    m_ui->m_comboBox->addItem(QLatin1String("Point"),       QVariant(QVariant::Point));
    m_ui->m_comboBox->addItem(QLatin1String("PointF"),      QVariant(QVariant::PointF));
    m_ui->m_comboBox->addItem(QLatin1String("Rect"),        QVariant(QVariant::Rect));
    m_ui->m_comboBox->addItem(QLatin1String("RectF"),       QVariant(QVariant::RectF));
    m_ui->m_comboBox->addItem(QLatin1String("Date"),        QVariant(QVariant::Date));
    m_ui->m_comboBox->addItem(QLatin1String("Time"),        QVariant(QVariant::Time));
    m_ui->m_comboBox->addItem(QLatin1String("DateTime"),    QVariant(QVariant::DateTime));
    m_ui->m_comboBox->addItem(QLatin1String("Font"),        QVariant(QVariant::Font));
    m_ui->m_comboBox->addItem(QLatin1String("Palette"),     QVariant(QVariant::Palette));
    m_ui->m_comboBox->addItem(QLatin1String("Color"),       QVariant(QVariant::Color));
    m_ui->m_comboBox->addItem(QLatin1String("Pixmap"),      QVariant(QVariant::Pixmap));
    m_ui->m_comboBox->addItem(QLatin1String("Icon"),        QVariant(QVariant::Icon));
    m_ui->m_comboBox->addItem(QLatin1String("Cursor"),      QVariant(QVariant::Cursor));
    m_ui->m_comboBox->addItem(QLatin1String("SizePolicy"),  QVariant(QVariant::SizePolicy));
    m_ui->m_comboBox->addItem(QLatin1String("KeySequence"), QVariant(QVariant::KeySequence));

    m_ui->m_comboBox->setCurrentIndex(0); // String
}

NewDynamicPropertyDialog::~NewDynamicPropertyDialog()
{
    delete m_ui;
}

void NewDynamicPropertyDialog::setReservedNames(const QStringList &names)
{
    m_reservedNames = names;
}

QString NewDynamicPropertyDialog::propertyName() const
{
    return m_ui->m_lineEdit->text();
}

QVariant NewDynamicPropertyDialog::propertyValue() const
{
    const int index = m_ui->m_comboBox->currentIndex();
    if (index == -1)
        return QVariant();
    return m_ui->m_comboBox->itemData(index);
}

void NewDynamicPropertyDialog::information(const QString &message)
{
    m_dialogGui->message(this, QDesignerDialogGuiInterface::PropertyEditorMessage, QMessageBox::Information, tr("Set Property Name"), message);
}

void NewDynamicPropertyDialog::on_m_buttonBox_clicked(QAbstractButton *btn)
{
    const int role = m_ui->m_buttonBox->buttonRole(btn);
    switch (role) {
        case QDialogButtonBox::RejectRole:
            reject();
            break;
        case QDialogButtonBox::AcceptRole:
            QString name = m_ui->m_lineEdit->text();
            if (m_reservedNames.contains(name)) {
                information(tr("The current object already has a property named '%1'.\nPlease select another, unique one.").arg(name));
                break;
            } else if (name.startsWith(QLatin1String("_q_"))) {
                information(tr("The '_q_' prefix is reserved for the Qt library.\nPlease select another name."));
                break;
            }
            accept();
            break;
    }
}

}

QT_END_NAMESPACE
