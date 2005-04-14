#include <QtGui>

#include "previewform.h"

PreviewForm::PreviewForm(QWidget *parent)
    : QDialog(parent)
{
    encodingComboBox = new QComboBox;
    populateEncodingComboBox();

    encodingLabel = new QLabel(tr("&Encoding:"));
    encodingLabel->setBuddy(encodingComboBox);

    textEdit = new QTextEdit;
    textEdit->setLineWrapMode(QTextEdit::NoWrap);
    textEdit->setReadOnly(true);

    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Cancel"));
    okButton->setDefault(true);

    connect(encodingComboBox, SIGNAL(activated(int)),
            this, SLOT(updateTextEdit()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(encodingLabel, 0, 0);
    mainLayout->addWidget(encodingComboBox, 0, 1);
    mainLayout->addWidget(textEdit, 1, 0, 1, 2);
    mainLayout->addLayout(buttonLayout, 2, 0, 1, 2);
    setLayout(mainLayout);

    setWindowTitle(tr("Choose Encoding"));
    resize(400, 300);
}

void PreviewForm::setEncodedData(const QByteArray &data)
{
    encodedData = data;
    updateTextEdit();
}

void PreviewForm::updateTextEdit()
{
    int mib = encodingComboBox->itemData(
                      encodingComboBox->currentIndex()).toInt();
    QTextCodec *codec = QTextCodec::codecForMib(mib);
    decodedStr = codec->toUnicode(encodedData);
    textEdit->setPlainText(decodedStr);
}

void PreviewForm::populateEncodingComboBox()
{
    foreach (int mib, QTextCodec::availableMibs()) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);
        encodingComboBox->addItem(codec->name(), mib);
    }
}
