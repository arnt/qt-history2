#include <QtGui>

#include "window.h"

Window::Window()
{
    QGroupBox *echoGroup = new QGroupBox(tr("Echo"), this);

    QLabel *echoLabel = new QLabel(tr("Mode:"), echoGroup);
    QComboBox *echoComboBox = new QComboBox(echoGroup);
    echoComboBox->addItem(tr("Normal"));
    echoComboBox->addItem(tr("Password"));
    echoComboBox->addItem(tr("No Echo"));

    echoLineEdit = new QLineEdit(echoGroup);
    echoLineEdit->setFocus();

    QGroupBox *validatorGroup = new QGroupBox(tr("Validator"), this);

    QLabel *validatorLabel = new QLabel(tr("Type:"), validatorGroup);
    QComboBox *validatorComboBox = new QComboBox(validatorGroup);
    validatorComboBox->addItem(tr("No validator"));
    validatorComboBox->addItem(tr("Integer validator"));
    validatorComboBox->addItem(tr("Double validator"));

    validatorLineEdit = new QLineEdit(validatorGroup);

    QGroupBox *alignmentGroup = new QGroupBox(tr("Alignment"), this);

    QLabel *alignmentLabel = new QLabel(tr("Type:"), alignmentGroup);
    QComboBox *alignmentComboBox = new QComboBox(alignmentGroup);
    alignmentComboBox->addItem(tr("Left"));
    alignmentComboBox->addItem(tr("Centered"));
    alignmentComboBox->addItem(tr("Right"));

    alignmentLineEdit = new QLineEdit(alignmentGroup);

    QGroupBox *inputMaskGroup = new QGroupBox(tr("Input mask"), this);

    QLabel *inputMaskLabel = new QLabel(tr("Type:"), inputMaskGroup);
    QComboBox *inputMaskComboBox = new QComboBox(inputMaskGroup);
    inputMaskComboBox->addItem(tr("No mask"));
    inputMaskComboBox->addItem(tr("Phone number"));
    inputMaskComboBox->addItem(tr("ISO date"));
    inputMaskComboBox->addItem(tr("License key"));

    inputMaskLineEdit = new QLineEdit(inputMaskGroup);

    QGroupBox *accessGroup = new QGroupBox(tr("Access"), this);

    QLabel *accessLabel = new QLabel(tr("Read-only:"), accessGroup);
    QComboBox *accessComboBox = new QComboBox(accessGroup);
    accessComboBox->addItem(tr("False"));
    accessComboBox->addItem(tr("True"));

    accessLineEdit = new QLineEdit(accessGroup);

    connect(echoComboBox, SIGNAL(activated(int)),
            this, SLOT(slotEchoChanged(int)));
    connect(validatorComboBox, SIGNAL(activated(int)),
            this, SLOT(slotValidatorChanged(int)));
    connect(alignmentComboBox, SIGNAL(activated(int)),
            this, SLOT(slotAlignmentChanged(int)));
    connect(inputMaskComboBox, SIGNAL(activated(int)),
            this, SLOT(slotInputMaskChanged(int)));
    connect(accessComboBox, SIGNAL(activated(int)),
            this, SLOT(slotAccessChanged(int)));

    QGridLayout *echoLayout = new QGridLayout(echoGroup);
    echoLayout->addWidget(echoLabel, 0, 0);
    echoLayout->addWidget(echoComboBox, 0, 1);
    echoLayout->addWidget(echoLineEdit, 1, 0, 1, 2);

    QGridLayout *validatorLayout = new QGridLayout(validatorGroup);
    validatorLayout->addWidget(validatorLabel, 0, 0);
    validatorLayout->addWidget(validatorComboBox, 0, 1);
    validatorLayout->addWidget(validatorLineEdit, 1, 0, 1, 2);

    QGridLayout *alignmentLayout = new QGridLayout(alignmentGroup);
    alignmentLayout->addWidget(alignmentLabel, 0, 0);
    alignmentLayout->addWidget(alignmentComboBox, 0, 1);
    alignmentLayout->addWidget(alignmentLineEdit, 1, 0, 1, 2);

    QGridLayout *inputMaskLayout = new QGridLayout(inputMaskGroup);
    inputMaskLayout->addWidget(inputMaskLabel, 0, 0);
    inputMaskLayout->addWidget(inputMaskComboBox, 0, 1);
    inputMaskLayout->addWidget(inputMaskLineEdit, 1, 0, 1, 2);

    QGridLayout *accessLayout = new QGridLayout(accessGroup);
    accessLayout->addWidget(accessLabel, 0, 0);
    accessLayout->addWidget(accessComboBox, 0, 1);
    accessLayout->addWidget(accessLineEdit, 1, 0, 1, 2);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(echoGroup);
    layout->addWidget(validatorGroup);
    layout->addWidget(alignmentGroup);
    layout->addWidget(inputMaskGroup);
    layout->addWidget(accessGroup);

    setWindowTitle(tr("Line Edits"));
}

void Window::slotEchoChanged(int index)
{
    switch (index) {
        case 0:
            echoLineEdit->setEchoMode(QLineEdit::Normal);
            break;
        case 1:
            echoLineEdit->setEchoMode(QLineEdit::Password);
            break;
        case 2:
    	    echoLineEdit->setEchoMode(QLineEdit::NoEcho);
            break;
        }
}

void Window::slotValidatorChanged(int index)
{
    switch (index) {
        case 0:
            validatorLineEdit->setValidator(0);
            break;
        case 1:
            validatorLineEdit->setValidator(new QIntValidator(
                validatorLineEdit));
            break;
        case 2:
            validatorLineEdit->setValidator(new QDoubleValidator(-999.0,
                999.0, 2, validatorLineEdit));
            break;
    }

    validatorLineEdit->setText("");
}

void Window::slotAlignmentChanged(int index)
{
    switch (index) {
        case 0:
            alignmentLineEdit->setAlignment(Qt::AlignLeft);
            break;
        case 1:
            alignmentLineEdit->setAlignment(Qt::AlignCenter);
            break;
        case 2:
    	    alignmentLineEdit->setAlignment(Qt::AlignRight);
            break;
        }
}

void Window::slotInputMaskChanged(int index)
{
    switch (index) {
        case 0:
            inputMaskLineEdit->setInputMask(QString::null);
            break;
        case 1:
            inputMaskLineEdit->setInputMask("+99 99 99 99 99;_");
            break;
        case 2:
            inputMaskLineEdit->setInputMask("0000-00-00");
            inputMaskLineEdit->setText("00000000");
            inputMaskLineEdit->setCursorPosition(0);
            break;
        case 3:
            inputMaskLineEdit->setInputMask(">AAAAA-AAAAA-AAAAA-AAAAA-AAAAA;#");
            break;
    }
}

void Window::slotAccessChanged(int index)
{
    switch (index) {
    case 0:
        accessLineEdit->setReadOnly(false);
        break;
    case 1:
        accessLineEdit->setReadOnly(true);
        break;
    }
}
