#include <QtGui>

#include "window.h"

Window::Window()
{
    QGroupBox *echoGroup = new QGroupBox(tr("Echo"));

    QLabel *echoLabel = new QLabel(tr("Mode:"));
    QComboBox *echoComboBox = new QComboBox;
    echoComboBox->addItem(tr("Normal"));
    echoComboBox->addItem(tr("Password"));
    echoComboBox->addItem(tr("No Echo"));

    echoLineEdit = new QLineEdit;
    echoLineEdit->setFocus();

    QGroupBox *validatorGroup = new QGroupBox(tr("Validator"));

    QLabel *validatorLabel = new QLabel(tr("Type:"));
    QComboBox *validatorComboBox = new QComboBox;
    validatorComboBox->addItem(tr("No validator"));
    validatorComboBox->addItem(tr("Integer validator"));
    validatorComboBox->addItem(tr("Double validator"));

    validatorLineEdit = new QLineEdit;

    QGroupBox *alignmentGroup = new QGroupBox(tr("Alignment"));

    QLabel *alignmentLabel = new QLabel(tr("Type:"));
    QComboBox *alignmentComboBox = new QComboBox;
    alignmentComboBox->addItem(tr("Left"));
    alignmentComboBox->addItem(tr("Centered"));
    alignmentComboBox->addItem(tr("Right"));

    alignmentLineEdit = new QLineEdit;

    QGroupBox *inputMaskGroup = new QGroupBox(tr("Input mask"));

    QLabel *inputMaskLabel = new QLabel(tr("Type:"));
    QComboBox *inputMaskComboBox = new QComboBox;
    inputMaskComboBox->addItem(tr("No mask"));
    inputMaskComboBox->addItem(tr("Phone number"));
    inputMaskComboBox->addItem(tr("ISO date"));
    inputMaskComboBox->addItem(tr("License key"));

    inputMaskLineEdit = new QLineEdit;

    QGroupBox *accessGroup = new QGroupBox(tr("Access"));

    QLabel *accessLabel = new QLabel(tr("Read-only:"));
    QComboBox *accessComboBox = new QComboBox;
    accessComboBox->addItem(tr("False"));
    accessComboBox->addItem(tr("True"));

    accessLineEdit = new QLineEdit;

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

    QGridLayout *echoLayout = new QGridLayout;
    echoLayout->addWidget(echoLabel, 0, 0);
    echoLayout->addWidget(echoComboBox, 0, 1);
    echoLayout->addWidget(echoLineEdit, 1, 0, 1, 2);
    echoGroup->setLayout(echoLayout);

    QGridLayout *validatorLayout = new QGridLayout;
    validatorLayout->addWidget(validatorLabel, 0, 0);
    validatorLayout->addWidget(validatorComboBox, 0, 1);
    validatorLayout->addWidget(validatorLineEdit, 1, 0, 1, 2);
    validatorGroup->setLayout(validatorLayout);

    QGridLayout *alignmentLayout = new QGridLayout;
    alignmentLayout->addWidget(alignmentLabel, 0, 0);
    alignmentLayout->addWidget(alignmentComboBox, 0, 1);
    alignmentLayout->addWidget(alignmentLineEdit, 1, 0, 1, 2);
    alignmentGroup-> setLayout(alignmentLayout);

    QGridLayout *inputMaskLayout = new QGridLayout;
    inputMaskLayout->addWidget(inputMaskLabel, 0, 0);
    inputMaskLayout->addWidget(inputMaskComboBox, 0, 1);
    inputMaskLayout->addWidget(inputMaskLineEdit, 1, 0, 1, 2);
    inputMaskGroup->setLayout(inputMaskLayout);

    QGridLayout *accessLayout = new QGridLayout;
    accessLayout->addWidget(accessLabel, 0, 0);
    accessLayout->addWidget(accessComboBox, 0, 1);
    accessLayout->addWidget(accessLineEdit, 1, 0, 1, 2);
    accessGroup->setLayout(accessLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(echoGroup);
    layout->addWidget(validatorGroup);
    layout->addWidget(alignmentGroup);
    layout->addWidget(inputMaskGroup);
    layout->addWidget(accessGroup);
    setLayout(layout);

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
