#include "regexpdialog.h"

RegExpDialog::RegExpDialog(QWidget *parent)
    : QDialog(parent)
{
    patternComboBox = new QComboBox(this);
    patternComboBox->setEditable(true);
    patternComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    patternComboBox->addItem(tr("([A-Za-z_])([A-Za-z_0-9]*)"));

    patternLabel = new QLabel(tr("&Pattern:"), this);
    patternLabel->setBuddy(patternComboBox);

    textComboBox = new QComboBox(this);
    textComboBox->setEditable(true);
    textComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    textComboBox->addItem(tr("(10 + delta4) * 32"));

    textLabel = new QLabel(tr("&Text:"), this);
    textLabel->setBuddy(textComboBox);

    caseSensitiveCheckBox = new QCheckBox(tr("Case &Sensitive"), this);
    caseSensitiveCheckBox->setChecked(true);
    minimalCheckBox = new QCheckBox(tr("&Minimal"), this);
    wildcardCheckBox = new QCheckBox(tr("&Wildcard"), this);

    indexLabel = new QLabel(tr("Index of Match:"), this);
    indexEdit = new QLineEdit(this);
    indexEdit->setReadOnly(true);

    matchedLengthLabel = new QLabel(tr("Matched Length:"), this);
    matchedLengthEdit = new QLineEdit(this);
    matchedLengthEdit->setReadOnly(true);

    for (int i = 0; i < MaxCaptures; ++i) {
        captureLabels[i] = new QLabel(tr("Capture %1:").arg(i), this);
        captureEdits[i] = new QLineEdit(this);
        captureEdits[i]->setReadOnly(true);
    }
    captureLabels[0]->setText(tr("Match:"));

    copyButton = new QPushButton(tr("&Copy"), this);
    quitButton = new QPushButton(tr("&Quit"), this);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(patternLabel, 0, 0);
    gridLayout->addWidget(patternComboBox, 0, 1);
    gridLayout->addWidget(textLabel, 1, 0);
    gridLayout->addWidget(textComboBox, 1, 1);

    QHBoxLayout *checkboxLayout = new QHBoxLayout;
    checkboxLayout->addWidget(caseSensitiveCheckBox);
    checkboxLayout->addWidget(minimalCheckBox);
    checkboxLayout->addWidget(wildcardCheckBox);
    checkboxLayout->addStretch(1);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(copyButton);
    buttonLayout->addWidget(quitButton);

    QGridLayout *middleLayout = new QGridLayout;
    middleLayout->addWidget(indexLabel, 0, 0);
    middleLayout->addWidget(indexEdit, 0, 1);
    middleLayout->addWidget(matchedLengthLabel, 1, 0);
    middleLayout->addWidget(matchedLengthEdit, 1, 1);

    for (int j = 0; j < MaxCaptures; ++j) {
        middleLayout->addWidget(captureLabels[j], j + 2, 0);
        middleLayout->addWidget(captureEdits[j], j + 2, 1);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(gridLayout);
    mainLayout->addLayout(checkboxLayout);
    mainLayout->addLayout(middleLayout);
    mainLayout->addLayout(buttonLayout);

    connect(patternComboBox, SIGNAL(editTextChanged(QString)), this, SLOT(refresh()));
    connect(textComboBox, SIGNAL(editTextChanged(QString)), this, SLOT(refresh()));
    connect(caseSensitiveCheckBox, SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(minimalCheckBox, SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(wildcardCheckBox, SIGNAL(toggled(bool)), this, SLOT(refresh()));

    connect(copyButton, SIGNAL(clicked()), this, SLOT(copy()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    setWindowTitle(tr("RegExp"));
    refresh();
}

void RegExpDialog::refresh()
{
    QString pattern = patternComboBox->currentText();
    QString text = textComboBox->currentText();

    QRegExp rx(pattern);
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    if (caseSensitiveCheckBox->isChecked())
        cs = Qt::CaseSensitive;
    rx.setCaseSensitivity(cs);
    rx.setMinimal(minimalCheckBox->isChecked());
    QRegExp::PatternSyntax syntax =
            wildcardCheckBox->isChecked() ? QRegExp::Wildcard : QRegExp::RegExp;
    rx.setPatternSyntax(syntax);

    QPalette pal = patternComboBox->palette();
    if (rx.isValid())
        pal.setColor(QPalette::Text, palette().color(QPalette::Text));
    else
        pal.setColor(QPalette::Text, Qt::red);
    patternComboBox->setPalette(pal);

    indexEdit->setText(QString::number(rx.indexIn(text)));
    matchedLengthEdit->setText(QString::number(rx.matchedLength()));
    for (int i = 0; i < MaxCaptures; ++i) {
        captureLabels[i]->setEnabled(i <= rx.numCaptures());
        captureEdits[i]->setEnabled(i <= rx.numCaptures());
        captureEdits[i]->setText(rx.cap(i));
    }
}

void RegExpDialog::copy()
{
    QString escaped = patternComboBox->currentText();
    escaped = escaped.replace("\\", "\\\\");
    escaped = "\"" + escaped + "\"";

    QClipboard *cb = QApplication::clipboard();
    cb->setText(escaped, QClipboard::Clipboard);
    if (cb->supportsSelection())
	cb->setText(escaped, QClipboard::Selection);
}
