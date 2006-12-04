#include "newdynamicpropertydialog.h"
#include "ui_newdynamicpropertydialog.h"
#include <QMessageBox>

using namespace qdesigner_internal;

NewDynamicPropertyDialog::NewDynamicPropertyDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui = new Ui::NewDynamicPropertyDialog;
    m_ui->setupUi(this);

    m_nameToValue[tr("String")] = QVariant(QVariant::String);
    m_nameToValue[tr("StringList")] = QVariant(QVariant::StringList);
    m_nameToValue[tr("Char")] = QVariant(QVariant::Char);
    m_nameToValue[tr("ByteArray")] = QVariant(QVariant::ByteArray);
    m_nameToValue[tr("Url")] = QVariant(QVariant::Url);
    m_nameToValue[tr("Bool")] = QVariant(QVariant::Bool);
    m_nameToValue[tr("Int")] = QVariant(QVariant::Int);
    m_nameToValue[tr("UInt")] = QVariant(QVariant::UInt);
    m_nameToValue[tr("LongLong")] = QVariant(QVariant::LongLong);
    m_nameToValue[tr("ULongLong")] = QVariant(QVariant::ULongLong);
    m_nameToValue[tr("Double")] = QVariant(QVariant::Double);
    m_nameToValue[tr("Size")] = QVariant(QVariant::Size);
    m_nameToValue[tr("SizeF")] = QVariant(QVariant::SizeF);
    m_nameToValue[tr("Point")] = QVariant(QVariant::Point);
    m_nameToValue[tr("PointF")] = QVariant(QVariant::PointF);
    m_nameToValue[tr("Rect")] = QVariant(QVariant::Rect);
    m_nameToValue[tr("RectF")] = QVariant(QVariant::RectF);
    m_nameToValue[tr("Date")] = QVariant(QVariant::Date);
    m_nameToValue[tr("Time")] = QVariant(QVariant::Time);
    m_nameToValue[tr("DateTime")] = QVariant(QVariant::DateTime);
    m_nameToValue[tr("Font")] = QVariant(QVariant::Font);
    m_nameToValue[tr("Palette")] = QVariant(QVariant::Palette);
    m_nameToValue[tr("Color")] = QVariant(QVariant::Color);
    m_nameToValue[tr("Pixmap")] = QVariant(QVariant::Pixmap);
    m_nameToValue[tr("Icon")] = QVariant(QVariant::Icon);
    m_nameToValue[tr("Cursor")] = QVariant(QVariant::Cursor);
    m_nameToValue[tr("SizePolicy")] = QVariant(QVariant::SizePolicy);
    m_nameToValue[tr("KeySequence")] = QVariant(QVariant::KeySequence);

    int idx = 0;
    QMap<QString, QVariant>::const_iterator it = m_nameToValue.constBegin();
    while (it != m_nameToValue.constEnd()) {
        m_ui->m_comboBox->addItem(it.key());
        if (it.value() == QVariant(QVariant::String))
            idx = m_ui->m_comboBox->count() - 1;
        it++;
    }
    m_ui->m_comboBox->setCurrentIndex(idx);
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
    return m_nameToValue.value(m_ui->m_comboBox->currentText());
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
                QMessageBox::information(this, tr("Set Property Name"), tr("Current object already has a property '%1'. Please select another, unique one.").arg(name));
                    break;
            }
            accept();
            break;
    }
}
