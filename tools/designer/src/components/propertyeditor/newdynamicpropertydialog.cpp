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
#include <QMessageBox>

namespace qdesigner_internal {

NewDynamicPropertyDialog::NewDynamicPropertyDialog(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui::NewDynamicPropertyDialog)
{
    m_ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    const NameToValueMap::const_iterator mcend = nameToValueMap().constEnd();
    int idx = 0;
    for (NameToValueMap::const_iterator it = nameToValueMap().constBegin(); it != mcend; ++it) {
        if (it.value() == QVariant(QVariant::String))
            idx = m_ui->m_comboBox->count();
        m_ui->m_comboBox->addItem(it.key());
    }
    m_ui->m_comboBox->setCurrentIndex(idx);
}

const NewDynamicPropertyDialog::NameToValueMap &NewDynamicPropertyDialog::nameToValueMap()
{
    static NameToValueMap rc;
    if (rc.empty()) {
        rc.insert(QLatin1String("String"),      QVariant(QVariant::String));
        rc.insert(QLatin1String("StringList"),  QVariant(QVariant::StringList));
        rc.insert(QLatin1String("Char"),        QVariant(QVariant::Char));
        rc.insert(QLatin1String("ByteArray"),   QVariant(QVariant::ByteArray));
        rc.insert(QLatin1String("Url"),         QVariant(QVariant::Url));
        rc.insert(QLatin1String("Bool"),        QVariant(QVariant::Bool));
        rc.insert(QLatin1String("Int"),         QVariant(QVariant::Int));
        rc.insert(QLatin1String("UInt"),        QVariant(QVariant::UInt));
        rc.insert(QLatin1String("LongLong"),    QVariant(QVariant::LongLong));
        rc.insert(QLatin1String("ULongLong"),   QVariant(QVariant::ULongLong));
        rc.insert(QLatin1String("Double"),      QVariant(QVariant::Double));
        rc.insert(QLatin1String("Size"),        QVariant(QVariant::Size));
        rc.insert(QLatin1String("SizeF"),       QVariant(QVariant::SizeF));
        rc.insert(QLatin1String("Point"),       QVariant(QVariant::Point));
        rc.insert(QLatin1String("PointF"),      QVariant(QVariant::PointF));
        rc.insert(QLatin1String("Rect"),        QVariant(QVariant::Rect));
        rc.insert(QLatin1String("RectF"),       QVariant(QVariant::RectF));
        rc.insert(QLatin1String("Date"),        QVariant(QVariant::Date));
        rc.insert(QLatin1String("Time"),        QVariant(QVariant::Time));
        rc.insert(QLatin1String("DateTime"),    QVariant(QVariant::DateTime));
        rc.insert(QLatin1String("Font"),        QVariant(QVariant::Font));
        rc.insert(QLatin1String("Palette"),     QVariant(QVariant::Palette));
        rc.insert(QLatin1String("Color"),       QVariant(QVariant::Color));
        rc.insert(QLatin1String("Pixmap"),      QVariant(QVariant::Pixmap));
        rc.insert(QLatin1String("Icon"),        QVariant(QVariant::Icon));
        rc.insert(QLatin1String("Cursor"),      QVariant(QVariant::Cursor));
        rc.insert(QLatin1String("SizePolicy"),  QVariant(QVariant::SizePolicy));
        rc.insert(QLatin1String("KeySequence"), QVariant(QVariant::KeySequence));
    }
    return rc;
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
    return nameToValueMap().value(m_ui->m_comboBox->currentText());
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
                QMessageBox::information(this, tr("Set Property Name"), tr("The current object already has a property named '%1'.\nPlease select another, unique one.").arg(name));
                    break;
            } else if (name.startsWith(QLatin1String("_q_"))) {
                QMessageBox::information(this, tr("Set Property Name"), tr("The '_q_' prefix is reserved for Qt library.\nPlease select another name."));
                    break;
            }
            accept();
            break;
    }
}

}
