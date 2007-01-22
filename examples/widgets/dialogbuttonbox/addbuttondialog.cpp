#include <QtGui>

#include "addbuttondialog.h"

AddButtonDialog::AddButtonDialog(QDialogButtonBox *box, QWidget *parent) 
    : QDialog(parent)
{
    dialogBox = box;

    setupUi(this); 
    fillStandardButtonCombo();
    fillCustomButtonCombo();
    standardButtonSelected(standardButtonCombo->currentText());

    connect(standardButtonCombo, SIGNAL(activated(const QString &)),
	    this, SLOT(standardButtonSelected(const QString &)));
    connect(okCancelDialogBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(okCancelDialogBox, SIGNAL(rejected()),
	    this, SLOT(reject()));
}

QString AddButtonDialog::roleToString(QDialogButtonBox::ButtonRole role)
{
    switch (role) {
	case QDialogButtonBox::AcceptRole:
	    return "Accept";
	case QDialogButtonBox::RejectRole:
	    return "Reject";
	case QDialogButtonBox::DestructiveRole:
	    return "Destructive";
	case QDialogButtonBox::ActionRole:
	    return "Action";
	case QDialogButtonBox::HelpRole:
	    return "Help";
	case QDialogButtonBox::YesRole:
	    return "Yes";
	case QDialogButtonBox::NoRole:
	    return "No";
	case QDialogButtonBox::ApplyRole:
	    return "Apply";
	case QDialogButtonBox::ResetRole:
	    return "Reset";
	default:
	    return "Invalid Role";
    }
}

void AddButtonDialog::addButton()
{
    QAbstractButton *addedButton = 0;

    if (tabWidget->tabText(tabWidget->currentIndex()) == 
	"Standard Button") {
	QDialogButtonBox::StandardButton standardButton =
	    QDialogButtonBox::StandardButton(
		standardButtonCombo->itemData(
		    standardButtonCombo->currentIndex()).value<int>());

	if (dialogBox->button(standardButton) == 0)
	    addedButton = dialogBox->addButton(standardButton);
    } else {
	addedButton = new QPushButton(customButtonLineEdit->text());

	if (!contains(addedButton, dialogBox->buttons()))
	    dialogBox->addButton(addedButton, QDialogButtonBox::ButtonRole(
		customButtonCombo->itemData(
		    customButtonCombo->currentIndex()).value<int>()));
    }

    if (addedButton)
	addedButton->setStyle(dialogBox->style());
}

void AddButtonDialog::standardButtonSelected(const QString & /* text */)
{
    int value = standardButtonCombo->itemData(
	standardButtonCombo->currentIndex()).toInt();
    
    QDialogButtonBox box;
    QAbstractButton *button = 
	box.addButton(QDialogButtonBox::StandardButton(value));
    standardRoleLabel->setText(roleToString(box.buttonRole(button)));
    standardTextLabel->setText(button->text().remove('&'));
}

void AddButtonDialog::fillStandardButtonCombo()
{
    QMetaEnum standardEnum = QDialogButtonBox::staticMetaObject.enumerator(0);
    QDialogButtonBox box;

    for (int i = 1; i < standardEnum.keyCount(); ++i) {
	QString key = standardEnum.key(i);
	
	QDialogButtonBox::StandardButton standardButton =
	    QDialogButtonBox::StandardButton(
		standardEnum.keyToValue(standardEnum.key(i)));
	standardButtonCombo->addItem(
	    box.addButton(standardButton)->text().remove('&'), 
			  int(standardButton));
    }    
}

void AddButtonDialog::fillCustomButtonCombo()
{
    customButtonCombo->addItem("Accept Role", 
			       int(QDialogButtonBox::AcceptRole));
    customButtonCombo->addItem("Reject Role", 
			       int(QDialogButtonBox::RejectRole));
    customButtonCombo->addItem("Destructive Role", 
			       int(QDialogButtonBox::DestructiveRole));
    customButtonCombo->addItem("Action Role", 
			       int(QDialogButtonBox::ActionRole));
    customButtonCombo->addItem("Help Role", int(QDialogButtonBox::HelpRole));
    customButtonCombo->addItem("Yes Role", int(QDialogButtonBox::YesRole));
    customButtonCombo->addItem("No Role", int(QDialogButtonBox::NoRole));
    customButtonCombo->addItem("Apply Role", int(QDialogButtonBox::ApplyRole));
    customButtonCombo->addItem("Reset Role", int(QDialogButtonBox::ResetRole));
}

bool AddButtonDialog::contains(QAbstractButton *button,
			       QList<QAbstractButton *> buttons)
{
    foreach (QAbstractButton *listButton, buttons) {
	if (button->text().remove('&') == listButton->text().remove('&'))
	    return true;
    }
    return false;
}
