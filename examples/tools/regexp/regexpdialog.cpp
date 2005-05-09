/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "regexpdialog.h"

RegExpDialog::RegExpDialog(QWidget *parent)
    : QDialog(parent)
{
    patternComboBox = new QComboBox;
    patternComboBox->setEditable(true);
    patternComboBox->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Preferred);

    patternLabel = new QLabel(tr("&Pattern:"));
    patternLabel->setBuddy(patternComboBox);

    escapedPatternLineEdit = new QLineEdit;
    escapedPatternLineEdit->setReadOnly(true);
    QPalette palette = escapedPatternLineEdit->palette();
    palette.setBrush(QPalette::Base, palette.brush(QPalette::Disabled, QPalette::Base));
    escapedPatternLineEdit->setPalette(palette);

    escapedPatternLabel = new QLabel(tr("&Escaped Pattern:"));
    escapedPatternLabel->setBuddy(escapedPatternLineEdit);

    textComboBox = new QComboBox;
    textComboBox->setEditable(true);
    textComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    textLabel = new QLabel(tr("&Text:"));
    textLabel->setBuddy(textComboBox);

    caseSensitiveCheckBox = new QCheckBox(tr("Case &Sensitive"));
    caseSensitiveCheckBox->setChecked(true);
    minimalCheckBox = new QCheckBox(tr("&Minimal"));
    wildcardCheckBox = new QCheckBox(tr("&Wildcard"));

    indexLabel = new QLabel(tr("Index of Match:"));
    indexEdit = new QLineEdit;
    indexEdit->setReadOnly(true);

    matchedLengthLabel = new QLabel(tr("Matched Length:"));
    matchedLengthEdit = new QLineEdit;
    matchedLengthEdit->setReadOnly(true);

    for (int i = 0; i < MaxCaptures; ++i) {
        captureLabels[i] = new QLabel(tr("Capture %1:").arg(i));
        captureEdits[i] = new QLineEdit;
        captureEdits[i]->setReadOnly(true);
    }
    captureLabels[0]->setText(tr("Match:"));

    QHBoxLayout *checkBoxLayout = new QHBoxLayout;
    checkBoxLayout->addWidget(caseSensitiveCheckBox);
    checkBoxLayout->addWidget(minimalCheckBox);
    checkBoxLayout->addWidget(wildcardCheckBox);
    checkBoxLayout->addStretch(1);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(patternLabel, 0, 0);
    mainLayout->addWidget(patternComboBox, 0, 1);
    mainLayout->addWidget(escapedPatternLabel, 1, 0);
    mainLayout->addWidget(escapedPatternLineEdit, 1, 1);
    mainLayout->addWidget(textLabel, 2, 0);
    mainLayout->addWidget(textComboBox, 2, 1);
    mainLayout->addLayout(checkBoxLayout, 3, 0, 1, 2);
    mainLayout->addWidget(indexLabel, 4, 0);
    mainLayout->addWidget(indexEdit, 4, 1);
    mainLayout->addWidget(matchedLengthLabel, 5, 0);
    mainLayout->addWidget(matchedLengthEdit, 5, 1);

    for (int j = 0; j < MaxCaptures; ++j) {
        mainLayout->addWidget(captureLabels[j], 6 + j, 0);
        mainLayout->addWidget(captureEdits[j], 6 + j, 1);
    }
    setLayout(mainLayout);

    connect(patternComboBox, SIGNAL(editTextChanged(const QString &)),
            this, SLOT(refresh()));
    connect(textComboBox, SIGNAL(editTextChanged(const QString &)),
            this, SLOT(refresh()));
    connect(caseSensitiveCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(refresh()));
    connect(minimalCheckBox, SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(wildcardCheckBox, SIGNAL(toggled(bool)), this, SLOT(refresh()));

    patternComboBox->addItem(tr("([A-Za-z_])([A-Za-z_0-9]*)"));
    textComboBox->addItem(tr("(10 + delta4) * 32"));

    setWindowTitle(tr("RegExp"));
    refresh();
}

void RegExpDialog::refresh()
{
    QString pattern = patternComboBox->currentText();
    QString text = textComboBox->currentText();

    QString escaped = patternComboBox->currentText();
    escaped = escaped.replace("\\", "\\\\");
    escaped = "\"" + escaped + "\"";
    escapedPatternLineEdit->setText(escaped);

    QRegExp rx(pattern);
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    if (caseSensitiveCheckBox->isChecked())
        cs = Qt::CaseSensitive;
    rx.setCaseSensitivity(cs);
    rx.setMinimal(minimalCheckBox->isChecked());
    QRegExp::PatternSyntax syntax =
            wildcardCheckBox->isChecked() ? QRegExp::Wildcard : QRegExp::RegExp;
    rx.setPatternSyntax(syntax);

    QPalette palette = patternComboBox->palette();
    if (rx.isValid()) {
        palette.setColor(QPalette::Text,
                         textComboBox->palette().color(QPalette::Text));
    } else {
        palette.setColor(QPalette::Text, Qt::red);
    }
    patternComboBox->setPalette(palette);

    indexEdit->setText(QString::number(rx.indexIn(text)));
    matchedLengthEdit->setText(QString::number(rx.matchedLength()));
    for (int i = 0; i < MaxCaptures; ++i) {
        captureLabels[i]->setEnabled(i <= rx.numCaptures());
        captureEdits[i]->setEnabled(i <= rx.numCaptures());
        captureEdits[i]->setText(rx.cap(i));
    }
}
