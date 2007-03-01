/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LICENSEWIZARD_H
#define LICENSEWIZARD_H

#include <QWizard>

class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;

class LicenseWizard : public QWizard
{
    Q_OBJECT

public:
    enum { Page_Intro, Page_Evaluate, Page_Register, Page_Details,
           Page_Conclusion };

    LicenseWizard(QWidget *parent = 0);

private slots:
    void showHelp();
};

class IntroPage : public QWizardPage
{
    Q_OBJECT

public:
    IntroPage(QWidget *parent = 0);

    int nextId() const;

private:
    QLabel *topLabel;
    QRadioButton *registerRadioButton;
    QRadioButton *evaluateRadioButton;
};

class EvaluatePage : public QWizardPage
{
    Q_OBJECT

public:
    EvaluatePage(QWidget *parent = 0);

    int nextId() const;

private:
    QLabel *nameLabel;
    QLabel *emailLabel;
    QLineEdit *nameLineEdit;
    QLineEdit *emailLineEdit;
};

class RegisterPage : public QWizardPage
{
    Q_OBJECT

public:
    RegisterPage(QWidget *parent = 0);

    int nextId() const;

private:
    QLabel *nameLabel;
    QLabel *upgradeKeyLabel;
    QLineEdit *nameLineEdit;
    QLineEdit *upgradeKeyLineEdit;
};

class DetailsPage : public QWizardPage
{
    Q_OBJECT

public:
    DetailsPage(QWidget *parent = 0);

    int nextId() const;

private:
    QLabel *companyLabel;
    QLabel *emailLabel;
    QLabel *postalLabel;
    QLineEdit *companyLineEdit;
    QLineEdit *emailLineEdit;
    QLineEdit *postalLineEdit;
};

class ConclusionPage : public QWizardPage
{
    Q_OBJECT

public:
    ConclusionPage(QWidget *parent = 0);

    void initializePage();
    int nextId() const;
    void setVisible(bool visible);

private slots:
    void printButtonClicked();

private:
    QLabel *bottomLabel;
    QCheckBox *agreeCheckBox;
};

#endif
