#include "newdynamicpropertydialog.h"
#include "ui_newdynamicpropertydialog.h"
#include <QMessageBox>

namespace qdesigner_internal {

NewDynamicPropertyDialog::NewDynamicPropertyDialog(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui::NewDynamicPropertyDialog)
{
    m_ui->setupUi(this);

    const NameToValueMap::const_iterator mcend = nameToValueMap().constEnd();
    int idx = 0;
    for (NameToValueMap::const_iterator it = nameToValueMap().constBegin(); it != mcend; ++it) {
        if (it.value() == QVariant(QVariant::String))
            idx = m_ui->m_comboBox->count() - 1;
        m_ui->m_comboBox->addItem(it.key());
    }
    m_ui->m_comboBox->setCurrentIndex(idx);
}

const NewDynamicPropertyDialog::NameToValueMap &NewDynamicPropertyDialog::nameToValueMap()
{
    static NameToValueMap rc;
    if (rc.empty()) {
        rc.insert(tr("String"),      QVariant(QVariant::String));
        rc.insert(tr("StringList"),  QVariant(QVariant::StringList));
        rc.insert(tr("Char"),        QVariant(QVariant::Char));
        rc.insert(tr("ByteArray"),   QVariant(QVariant::ByteArray));
        rc.insert(tr("Url"),         QVariant(QVariant::Url));
        rc.insert(tr("Bool"),        QVariant(QVariant::Bool));
        rc.insert(tr("Int"),         QVariant(QVariant::Int));
        rc.insert(tr("UInt"),        QVariant(QVariant::UInt));
        rc.insert(tr("LongLong"),    QVariant(QVariant::LongLong));
        rc.insert(tr("ULongLong"),   QVariant(QVariant::ULongLong));
        rc.insert(tr("Double"),      QVariant(QVariant::Double));
        rc.insert(tr("Size"),        QVariant(QVariant::Size));
        rc.insert(tr("SizeF"),       QVariant(QVariant::SizeF));
        rc.insert(tr("Point"),       QVariant(QVariant::Point));
        rc.insert(tr("PointF"),      QVariant(QVariant::PointF));
        rc.insert(tr("Rect"),        QVariant(QVariant::Rect));
        rc.insert(tr("RectF"),       QVariant(QVariant::RectF));
        rc.insert(tr("Date"),        QVariant(QVariant::Date));
        rc.insert(tr("Time"),        QVariant(QVariant::Time));
        rc.insert(tr("DateTime"),    QVariant(QVariant::DateTime));
        rc.insert(tr("Font"),        QVariant(QVariant::Font));
        rc.insert(tr("Palette"),     QVariant(QVariant::Palette));
        rc.insert(tr("Color"),       QVariant(QVariant::Color));
        rc.insert(tr("Pixmap"),      QVariant(QVariant::Pixmap));
        rc.insert(tr("Icon"),        QVariant(QVariant::Icon));
        rc.insert(tr("Cursor"),      QVariant(QVariant::Cursor));
        rc.insert(tr("SizePolicy"),  QVariant(QVariant::SizePolicy));
        rc.insert(tr("KeySequence"), QVariant(QVariant::KeySequence));
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
            }
            accept();
            break;
    }
}

}
