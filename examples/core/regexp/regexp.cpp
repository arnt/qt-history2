#include "regexp.h"

const QString RESULT_TABLE =
    QObject::tr("<table bgcolor=\"white\" border=\"1\" "
                "cellspacing=\"1\" cellpadding=\"1\">"
                "<tr><td>Regexp</td>"
                "<td align=\"right\" colspan=\"2\"><b>%1</b></td></tr>"
                "<tr><td>Offset</td>"
                "<td align=\"right\" colspan=\"2\"><b>%2</b></td></tr>"
                "<tr><td>Captures</td>"
                "<td align=\"right\" colspan=\"2\"><b>%3</b></td></tr>");

const QString SUBHEADING =
    QObject::tr("<tr><td></td><td align=\"center\">Text</td>"
                "<td align=\"center\">Characters</td></tr>");

const QString MATCH =
    QObject::tr("<tr><td>Match</td><td><b>%1</b></td>"
                "<td align=\"right\"><b>%2</b></td></tr>");

const QString CAPTURE_ROW =
    QObject::tr("<tr><td>Capture #%1</td><td align=\"right\"><b>%2</b></td>"
                "<td align=\"right\"><b>%3</b></td></tr>");

const QString TABLE_END = QObject::tr("</table>");



Regexp::Regexp(QWidget* parent)
    : QDialog(parent)
{
    regexLabel = new QLabel(this);
    regexComboBox = new QComboBox(this);
    regexComboBox->setEditable(true);
    regexComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    regexComboBox->setToolTip(
            tr("Enter a regular expression here."
               "<br>(Don't double-up backslashes; "
               "Regexp does that automatically.)"));
    regexLabel->setBuddy(regexComboBox);
    textLabel = new QLabel(this);
    textComboBox = new QComboBox(this);
    textComboBox->setEditable(true);
    textComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    textComboBox->setToolTip(tr("Enter some text to match against."));
    textLabel->setBuddy(textComboBox);
    caseSensitiveCheckBox = new QCheckBox(this);
    caseSensitiveCheckBox->setChecked(true);
    caseSensitiveCheckBox->setToolTip(
            tr("Check this checkbox for case-sensitive matching; "
               "uncheck it for case-insensitive matching."));
    minimalCheckBox = new QCheckBox(this);
    minimalCheckBox->setToolTip(
            tr("Check this checkbox for minimal matching; "
               "uncheck it for maximal matching."));
    wildcardCheckBox = new QCheckBox(this);
    wildcardCheckBox->setToolTip(
            tr("Check this checkbox for wildcard matching "
               "(this is similar to file globbing); "
               "uncheck it for full regular expression matching."));
    resultLabel = new QLabel(this);
    resultLabel->setToolTip(
            tr("This shows the results of executing a regular expression "
               "on the given text."));
    executePushButton = new QPushButton(this);
    executePushButton->setDefault(true);
    executePushButton->setToolTip(
            tr("Click to see the results of searching the text for a match."));
    copyPushButton = new QPushButton(this);
    copyPushButton->setToolTip(
            tr("Click to copy the regular expression to the clipboard; "
               "backslashes are automatically doubled-up for pasting "
               "into your source code)."));
    quitPushButton = new QPushButton(this);
    quitPushButton->setToolTip(tr("Click to exit the application."));
    statusBar = new QStatusBar(this);

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
    buttonLayout->addWidget(executePushButton);
    buttonLayout->addWidget(copyPushButton);
    buttonLayout->addWidget(quitPushButton);
    buttonLayout->addStretch(1);
    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addWidget(resultLabel, 1);
    middleLayout->addLayout(buttonLayout);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(gridLayout);
    mainLayout->addLayout(checkboxLayout);
    mainLayout->addLayout(middleLayout);
    mainLayout->addWidget(statusBar);

    resize(QSize(500, 350).expandedTo(minimumSizeHint()));

    languageChange();

    connect(copyPushButton, SIGNAL(clicked()), this, SLOT(copy()));
    connect(executePushButton, SIGNAL(clicked()), this, SLOT(execute()));
    connect(quitPushButton, SIGNAL(clicked()), this, SLOT(accept()));

    execute();
    statusBar->message(tr("Hover the mouse over widgets for tool tips."));
}


void Regexp::execute()
{
    QString regex = regexComboBox->currentText();
    QString text = textComboBox->currentText();
    if (!regex.isEmpty() && !text.isEmpty()) {
	QRegExp re(regex);
        Qt::CaseSensitivity cs = Qt::CaseInsensitive;
        if (caseSensitiveCheckBox->isChecked())
            cs = Qt::CaseSensitive;
	re.setCaseSensitivity(cs);
	re.setMinimalMatching(minimalCheckBox->isChecked());
        QRegExp::PatternSyntax syntax = QRegExp::RegExp;
        if (wildcardCheckBox->isChecked())
            syntax = QRegExp::Wildcard;
	re.setPatternSyntax(syntax);
	if (!re.isValid()) {
	    statusBar->message(tr("Invalid regular expression: %1")
				.arg(re.errorString()));
	    return;
	}
	int offset = re.indexIn(text);
	int captures = re.numCaptures();
        QString result = RESULT_TABLE;
	QString escaped = regex.replace("\\", "\\\\");
        result = result.arg(escaped);
	if (offset != -1) {
            result = result.arg(offset) + SUBHEADING;
	    if (syntax != QRegExp::RegExp)
                captures = 0;
            result = result.arg(captures);
            result += MATCH.arg(re.cap(0)).arg(re.matchedLength());
	    if (syntax == QRegExp::RegExp) {
		for (int i = 1; i <= captures; ++i)
                    result += CAPTURE_ROW
                                .arg(i).arg(re.cap(i)).arg(re.cap(i).length());
	    }
	}
	else {
            result = result.arg(offset).arg(0);
            result += tr("<tr><td colspan=\"3\">No matches</td></tr>");
        }
        result += TABLE_END;
        resultLabel->setText(result);
	statusBar->message(tr("Executed \"%1\" on \"%2\"")
				.arg(escaped).arg(text));
    }
    else
	statusBar->message(tr("A regular expression and a text must be given"));
}


void Regexp::copy()
{
    QString escaped = regexComboBox->currentText();
    if (!escaped.isEmpty()) {
	escaped = escaped.replace("\\", "\\\\");
	QClipboard *cb = QApplication::clipboard();
	cb->setText(escaped, QClipboard::Clipboard);
	if (cb->supportsSelection())
	    cb->setText(escaped, QClipboard::Selection);
	statusBar->message(tr("Copied \"%1\" to the clipboard")
				.arg(escaped));
    }
}


void Regexp::languageChange()
{
    setWindowTitle(tr("Regexp"));
    regexLabel->setText(tr("&Regexp:"));
    regexComboBox->insertItem(tr("[A-Z]+=(\\d+):(\\d*)"));
    textLabel->setText(tr("&Text:"));
    textComboBox->insertItem(tr("ABC=12:3456"));
    caseSensitiveCheckBox->setText(tr("Case &Sensitive"));
    minimalCheckBox->setText(tr("&Minimal"));
    wildcardCheckBox->setText(tr("&Wildcard"));
    copyPushButton->setText(tr("&Copy"));
    executePushButton->setText(tr("&Execute"));
    quitPushButton->setText(tr("&Quit"));
}

