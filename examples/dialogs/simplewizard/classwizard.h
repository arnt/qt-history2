#ifndef CLASSWIZARD_H
#define CLASSWIZARD_H

#include "simplewizard.h"

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class FirstPage;
class SecondPage;
class ThirdPage;

class ClassWizard : public SimpleWizard
{
    Q_OBJECT

public:
    ClassWizard(QWidget *parent = 0);

protected:
    QWidget *createPage(int index);
    void accept();

private slots:
    void classNameChanged();

public:
    FirstPage *firstPage;
    SecondPage *secondPage;
    ThirdPage *thirdPage;
};

class FirstPage : public QWidget
{
    Q_OBJECT

public:
    FirstPage(ClassWizard *wizard);

    QLabel *topLabel;
    QLabel *classNameLabel;
    QLabel *baseClassLabel;
    QLineEdit *classNameLineEdit;
    QLineEdit *baseClassLineEdit;
    QCheckBox *qobjectMacroCheckBox;
    QGroupBox *groupBox;
    QRadioButton *qobjectCtorRadioButton;
    QRadioButton *qwidgetCtorRadioButton;
    QRadioButton *defaultCtorRadioButton;
    QCheckBox *copyCtorCheckBox;
};

class SecondPage : public QWidget
{
    Q_OBJECT

public:
    SecondPage(ClassWizard *wizard);

    QLabel *topLabel;
    QCheckBox *commentCheckBox;
    QCheckBox *protectCheckBox;
    QCheckBox *includeBaseCheckBox;
    QLabel *macroNameLabel;
    QLabel *baseIncludeLabel;
    QLineEdit *macroNameLineEdit;
    QLineEdit *baseIncludeLineEdit;
};

class ThirdPage : public QWidget
{
    Q_OBJECT

public:    
    ThirdPage(ClassWizard *wizard);

    QLabel *topLabel;
    QLabel *outputDirLabel;
    QLabel *headerLabel;
    QLabel *implementationLabel;
    QLineEdit *outputDirLineEdit;
    QLineEdit *headerLineEdit;
    QLineEdit *implementationLineEdit;
};

#endif
