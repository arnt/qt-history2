#include "regexpwindow.h"

static const char *ResultTable =
        "<table bgcolor=\"white\" border=\"1\" "
        "cellspacing=\"1\" cellpadding=\"1\">"
        "<tr><td>Offset</td>"
        "<td align=\"right\"><b>%1</b></td></tr>"
        "<tr><td>Matched Length</td>"
        "<td align=\"right\"><b>%2</b></td></tr>";

static const char *Match =
        "<tr><td>Match</td><td align=\"right\"><b>%1</b></td></tr>";

static const char *CaptureRow =
        "<tr><td>Capture %1</td><td align=\"right\"><b>%2</b></td></tr>";

static const char *TableEnd = "</table>";

RegExpWindow::RegExpWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *center = new QWidget(this);
    setCentralWidget(center);

    regexComboBox = new QComboBox(center);
    regexComboBox->setEditable(true);
    regexComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    regexComboBox->insertItem(tr("([A-Za-z_])([A-Za-z_0-9]*)"));

    regexLabel = new QLabel(tr("&Regular Expression:"), center);
    regexLabel->setBuddy(regexComboBox);

    textComboBox = new QComboBox(center);
    textComboBox->setEditable(true);
    textComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    textComboBox->setCurrentText(tr("(10 + x2) * 32"));

    textLabel = new QLabel(tr("&Text:"), center);
    textLabel->setBuddy(textComboBox);

    caseSensitiveCheckBox = new QCheckBox(tr("Case &Sensitive"), center);
    caseSensitiveCheckBox->setChecked(true);
    minimalCheckBox = new QCheckBox(tr("&Minimal"), center);
    wildcardCheckBox = new QCheckBox(tr("&Wildcard"), center);

    resultLabel = new QLabel(center);

    copyPushButton = new QPushButton(tr("&Copy"), center);

    quitPushButton = new QPushButton(tr("&Quit"), center);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(regexLabel, 0, 0);
    gridLayout->addWidget(regexComboBox, 0, 1);
    gridLayout->addWidget(textLabel, 1, 0);
    gridLayout->addWidget(textComboBox, 1, 1);

    QHBoxLayout *checkboxLayout = new QHBoxLayout;
    checkboxLayout->addWidget(caseSensitiveCheckBox);
    checkboxLayout->addWidget(minimalCheckBox);
    checkboxLayout->addWidget(wildcardCheckBox);
    checkboxLayout->addStretch(1);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(copyPushButton);
    buttonLayout->addWidget(quitPushButton);
    buttonLayout->addStretch(1);

    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addWidget(resultLabel, 1);
    middleLayout->addLayout(buttonLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(center);
    mainLayout->addLayout(gridLayout);
    mainLayout->addLayout(checkboxLayout);
    mainLayout->addLayout(middleLayout);

    connect(regexComboBox, SIGNAL(textChanged(QString)), this, SLOT(refresh()));
    connect(textComboBox, SIGNAL(textChanged(QString)), this, SLOT(refresh()));
    connect(caseSensitiveCheckBox, SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(minimalCheckBox, SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(wildcardCheckBox, SIGNAL(toggled(bool)), this, SLOT(refresh()));

    connect(copyPushButton, SIGNAL(clicked()), this, SLOT(copy()));
    connect(quitPushButton, SIGNAL(clicked()), this, SLOT(close()));

    setWindowTitle(tr("RegExp"));
    refresh();
}

void RegExpWindow::refresh()
{
    QString regex = regexComboBox->currentText();
    QString text = textComboBox->currentText();

    QRegExp re(regex);
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    if (caseSensitiveCheckBox->isChecked())
        cs = Qt::CaseSensitive;
    re.setCaseSensitivity(cs);
    re.setMinimalMatching(minimalCheckBox->isChecked());
    QRegExp::PatternSyntax syntax =
            wildcardCheckBox->isChecked() ? QRegExp::Wildcard : QRegExp::RegExp;
    if (!re.isValid()) {
	statusBar()->message(tr("Invalid regular expression"), 2000);
	return;
    }

    int offset = re.indexIn(text);
    int numCaptures = re.numCaptures();
    QString result = ResultTable;
    result = result.arg(offset).arg(re.matchedLength());
    if (offset != -1) {
        result += QString(Match).arg(re.cap(0));
        for (int i = 0; i < numCaptures; ++i)
            result += QString(CaptureRow).arg(i + 1).arg(re.cap(i + 1));
    }
    result += TableEnd;
    resultLabel->setText(result);
}

void RegExpWindow::copy()
{
    QString escaped = regexComboBox->currentText();
    if (!escaped.isEmpty()) {
	escaped = escaped.replace("\\", "\\\\");
	QClipboard *cb = QApplication::clipboard();
	cb->setText(escaped, QClipboard::Clipboard);
	if (cb->supportsSelection())
	    cb->setText(escaped, QClipboard::Selection);
	statusBar()->message(tr("Copied regular expression to the clipboard"), 2000);
    }
}
