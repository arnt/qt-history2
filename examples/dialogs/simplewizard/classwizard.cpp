#include <QtGui>

#include "classwizard.h"

ClassWizard::ClassWizard(QWidget *parent)
    : SimpleWizard(parent)
{
    setNumPages(3);
}

QWidget *ClassWizard::createPage(int index)
{
    switch (index) {
    case 0:
        firstPage = new FirstPage(this);
        return firstPage;
    case 1:
        secondPage = new SecondPage(this);
        return secondPage;
    case 2:
        thirdPage = new ThirdPage(this);
        return thirdPage;
    }
    return 0;
}

void ClassWizard::accept()
{
    QByteArray className = firstPage->classNameLineEdit->text().toAscii();
    QByteArray baseClass = firstPage->baseClassLineEdit->text().toAscii();
    bool qobjectMacro = firstPage->qobjectMacroCheckBox->isChecked();
    bool qobjectCtor = firstPage->qobjectCtorRadioButton->isChecked();
    bool qwidgetCtor = firstPage->qwidgetCtorRadioButton->isChecked();
    bool defaultCtor = firstPage->defaultCtorRadioButton->isChecked();
    bool copyCtor = firstPage->copyCtorCheckBox->isChecked();

    bool comment = secondPage->commentCheckBox->isChecked();
    bool protect = secondPage->protectCheckBox->isChecked();
    QByteArray macroName = secondPage->macroNameLineEdit->text().toAscii();
    bool includeBase = secondPage->includeBaseCheckBox->isChecked();
    QByteArray baseInclude = secondPage->baseIncludeLineEdit->text().toAscii();

    QString outputDir = thirdPage->outputDirLineEdit->text();
    QString header = thirdPage->headerLineEdit->text();
    QString implementation = thirdPage->implementationLineEdit->text();

    QByteArray block;

    if (comment) {
        block += "/*\n";
        block += "    " + header.toAscii() + "\n";
        block += "*/\n";
        block += "\n";
    }
    if (protect) {
        block += "#ifndef " + macroName + "\n";
        block += "#define " + macroName + "\n";
        block += "\n";
    }
    if (includeBase) {
        block += "#include " + baseInclude + "\n";
        block += "\n";
    }

    block += "class " + className;
    if (!baseClass.isEmpty())
        block += " : public " + baseClass;
    block += "\n";
    block += "{\n";

    /* qmake ignore Q_OBJECT */

    if (qobjectMacro) {
        block += "    Q_OBJECT\n";
        block += "\n";
    }
    block += "public:\n";

    if (qobjectCtor) {
        block += "    " + className + "(QObject *parent);\n";
    } else if (qwidgetCtor) {
        block += "    " + className + "(QWidget *parent);\n";
    } else if (defaultCtor) {
        block += "    " + className + "();\n";
        if (copyCtor) {
            block += "    " + className + "(const " + className + " &other);\n";
            block += "\n";
            block += "    " + className + " &operator=" + "(const " + className
                     + " &other);\n";
        }
    }
    block += "};\n";

    if (protect) {
        block += "\n";
        block += "#endif\n";
    }

    QFile headerFile(outputDir + "/" + header);
    if (!headerFile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Simple Wizard"),
                             tr("Cannot write file %1:\n%2")
                             .arg(headerFile.fileName())
                             .arg(headerFile.errorString()));
        return;
    }
    headerFile.write(block);

    block.clear();

    if (comment) {
        block += "/*\n";
        block += "    " + implementation.toAscii() + "\n";
        block += "*/\n";
        block += "\n";
    }
    block += "#include \"" + header.toAscii() + "\"\n";
    block += "\n";

    if (qobjectCtor) {
        block += className + "::" + className + "(QObject *parent)\n";
        block += "    : " + baseClass + "(parent)\n";
        block += "{\n";
        block += "}\n";
    } else if (qwidgetCtor) {
        block += className + "::" + className + "(QWidget *parent)\n";
        block += "    : " + baseClass + "(parent)\n";
        block += "{\n";
        block += "}\n";
    } else if (defaultCtor) {
        block += className + "::" + className + "()\n";
        block += "{\n";
        block += "    // missing code\n";
        block += "}\n";

        if (copyCtor) {
            block += "\n";
            block += className + "::" + className + "(const " + className
                     + " &other)\n";
            block += "{\n";
            block += "    *this = other;\n";
            block += "}\n";
            block += "\n";
            block += className + " &" + className + "::operator=(const "
                     + className + " &other)\n";
            block += "{\n";
            if (!baseClass.isEmpty())
                block += "    " + baseClass + "::operator=(other);\n";
            block += "    // missing code\n";
            block += "    return *this;\n";
            block += "}\n";
        }
    }

    QFile implementationFile(outputDir + "/" + implementation);
    if (!implementationFile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Simple Wizard"),
                             tr("Cannot write file %1:\n%2")
                             .arg(implementationFile.fileName())
                             .arg(implementationFile.errorString()));
        return;
    }
    implementationFile.write(block);

    QDialog::accept();
}

FirstPage::FirstPage(ClassWizard *wizard)
    : QWidget(wizard)
{
    topLabel = new QLabel(tr("<center><b>Class information</b></center>"
                             "<p>This wizard will generate a skeleton class "
                             "definition and member function definitions."),
                          this);
    topLabel->setWordWrap(false);

    classNameLabel = new QLabel(tr("Class &name:"), this);
    classNameLineEdit = new QLineEdit(this);
    classNameLabel->setBuddy(classNameLineEdit);
    setFocusProxy(classNameLineEdit);

    baseClassLabel = new QLabel(tr("&Base class:"), this);
    baseClassLineEdit = new QLineEdit(this);
    baseClassLabel->setBuddy(baseClassLineEdit);

    qobjectMacroCheckBox = new QCheckBox(tr("&Generate Q_OBJECT macro"), this);

    groupBox = new QGroupBox(tr("&Constructor"), this);

    qobjectCtorRadioButton = new QRadioButton(tr("&QObject-style constructor"),
                                              groupBox);
    qwidgetCtorRadioButton = new QRadioButton(tr("Q&Widget-style constructor"),
                                              groupBox);
    defaultCtorRadioButton = new QRadioButton(tr("&Default constructor"),
                                              groupBox);
    copyCtorCheckBox = new QCheckBox(tr("&Also generate copy constructor and "
                                        "assignment operator"), groupBox);

    defaultCtorRadioButton->setChecked(true);

    connect(classNameLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(classNameChanged()));
    connect(defaultCtorRadioButton, SIGNAL(toggled(bool)),
            copyCtorCheckBox, SLOT(setEnabled(bool)));

    wizard->setButtonEnabled(false);

    QVBoxLayout *groupBoxLayout = new QVBoxLayout(groupBox);
    groupBoxLayout->addWidget(qobjectCtorRadioButton);
    groupBoxLayout->addWidget(qwidgetCtorRadioButton);
    groupBoxLayout->addWidget(defaultCtorRadioButton);
    groupBoxLayout->addWidget(copyCtorCheckBox);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(topLabel, 0, 0, 1, 2);
    layout->setRowSpacing(1, 10);
    layout->addWidget(classNameLabel, 2, 0);
    layout->addWidget(classNameLineEdit, 2, 1);
    layout->addWidget(baseClassLabel, 3, 0);
    layout->addWidget(baseClassLineEdit, 3, 1);
    layout->addWidget(qobjectMacroCheckBox, 4, 0, 1, 2);
    layout->addWidget(groupBox, 5, 0, 1, 2);
    layout->setRowStretch(6, 1);
}

void FirstPage::classNameChanged()
{
    ClassWizard *wizard = qt_cast<ClassWizard *>(parent());
    wizard->setButtonEnabled(!classNameLineEdit->text().isEmpty());
}

SecondPage::SecondPage(ClassWizard *wizard)
    : QWidget(wizard)
{
    topLabel = new QLabel(tr("<center><b>Code style options</b></center>"),
                          this);

    commentCheckBox = new QCheckBox(tr("&Start generated files with a comment"),
                                    this);
    commentCheckBox->setChecked(true);
    setFocusProxy(commentCheckBox);

    protectCheckBox = new QCheckBox(tr("&Protect header file against multiple "
                                       "inclusions"), this);
    protectCheckBox->setChecked(true);

    macroNameLabel = new QLabel(tr("&Macro name:"), this);
    macroNameLineEdit = new QLineEdit(this);
    macroNameLabel->setBuddy(macroNameLineEdit);

    includeBaseCheckBox = new QCheckBox(tr("&Include base class definition"),
                                        this);
    baseIncludeLabel = new QLabel(tr("Base class include:"), this);
    baseIncludeLineEdit = new QLineEdit(this);
    baseIncludeLabel->setBuddy(baseIncludeLineEdit);

    QString className = wizard->firstPage->classNameLineEdit->text();
    macroNameLineEdit->setText(className.toUpper() + "_H");

    QString baseClass = wizard->firstPage->baseClassLineEdit->text();
    if (baseClass.isEmpty()) {
        includeBaseCheckBox->setEnabled(false);
        baseIncludeLabel->setEnabled(false);
        baseIncludeLineEdit->setEnabled(false);
    } else {
        includeBaseCheckBox->setChecked(true);
        if (QRegExp("Q[A-Z].*").exactMatch(baseClass)) {
            baseIncludeLineEdit->setText("<" + baseClass + ">");
        } else {
            baseIncludeLineEdit->setText("\"" + baseClass.toLower() + ".h\"");
        }
    }

    connect(protectCheckBox, SIGNAL(toggled(bool)),
            macroNameLabel, SLOT(setEnabled(bool)));
    connect(protectCheckBox, SIGNAL(toggled(bool)),
            macroNameLineEdit, SLOT(setEnabled(bool)));
    connect(includeBaseCheckBox, SIGNAL(toggled(bool)),
            baseIncludeLabel, SLOT(setEnabled(bool)));
    connect(includeBaseCheckBox, SIGNAL(toggled(bool)),
            baseIncludeLineEdit, SLOT(setEnabled(bool)));

    QGridLayout *layout = new QGridLayout(this);
    layout->setColumnSpacing(0, 20);
    layout->addWidget(topLabel, 0, 0, 1, 3);
    layout->setRowSpacing(1, 10);
    layout->addWidget(commentCheckBox, 2, 0, 1, 3);
    layout->addWidget(protectCheckBox, 3, 0, 1, 3);
    layout->addWidget(macroNameLabel, 4, 1);
    layout->addWidget(macroNameLineEdit, 4, 2);
    layout->addWidget(includeBaseCheckBox, 5, 0, 1, 3);
    layout->addWidget(baseIncludeLabel, 6, 1);
    layout->addWidget(baseIncludeLineEdit, 6, 2);
    layout->setRowStretch(7, 1);
}

ThirdPage::ThirdPage(ClassWizard *wizard)
    : QWidget(wizard)
{
    topLabel = new QLabel(tr("<center><b>Output files</b></center>"), this);

    outputDirLabel = new QLabel(tr("&Output directory:"), this);
    outputDirLineEdit = new QLineEdit(this);
    outputDirLabel->setBuddy(outputDirLineEdit);
    setFocusProxy(outputDirLineEdit);

    headerLabel = new QLabel(tr("&Header file name:"), this);
    headerLineEdit = new QLineEdit(this);
    headerLabel->setBuddy(headerLineEdit);

    implementationLabel = new QLabel(tr("&Implementation file name:"), this);
    implementationLineEdit = new QLineEdit(this);
    implementationLabel->setBuddy(implementationLineEdit);

    QString className = wizard->firstPage->classNameLineEdit->text();
    headerLineEdit->setText(className.toLower() + ".h");
    implementationLineEdit->setText(className.toLower() + ".cpp");
    outputDirLineEdit->setText(QDir::convertSeparators(QDir::homePath()));

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(topLabel, 0, 0, 1, 2);
    layout->setRowSpacing(1, 10);
    layout->addWidget(outputDirLabel, 2, 0);
    layout->addWidget(outputDirLineEdit, 2, 1);
    layout->addWidget(headerLabel, 3, 0);
    layout->addWidget(headerLineEdit, 3, 1);
    layout->addWidget(implementationLabel, 4, 0);
    layout->addWidget(implementationLineEdit, 4, 1);
    layout->setRowStretch(5, 1);
}
