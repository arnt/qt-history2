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

#ifndef LICENSEWIZARD_H
#define LICENSEWIZARD_H

#include "complexwizard.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class DetailsPage;
class EvaluatePage;
class FinishPage;
class RegisterPage;
class TitlePage;

class LicenseWizard : public ComplexWizard
{
public:
    LicenseWizard(QWidget *parent = 0);

private:
    TitlePage *titlePage;
    EvaluatePage *evaluatePage;
    RegisterPage *registerPage;
    DetailsPage *detailsPage;
    FinishPage *finishPage;

    friend class DetailsPage;
    friend class EvaluatePage;
    friend class FinishPage;
    friend class RegisterPage;
    friend class TitlePage;
};

class LicenseWizardPage : public WizardPage
{
public:
    LicenseWizardPage(LicenseWizard *wizard)
        : WizardPage(wizard), wizard(wizard) {}

protected:
    LicenseWizard *wizard;
};

class TitlePage : public LicenseWizardPage
{
public:
    TitlePage(LicenseWizard *wizard);

    void resetPage();
    WizardPage *nextPage();

private:
    QLabel *topLabel;
    QRadioButton *registerRadioButton;
    QRadioButton *evaluateRadioButton;
};

class EvaluatePage : public LicenseWizardPage
{
public:
    EvaluatePage(LicenseWizard *wizard);

    void resetPage();
    WizardPage *nextPage();
    bool isComplete();

private:
    QLabel *topLabel;
    QLabel *nameLabel;
    QLabel *emailLabel;
    QLabel *bottomLabel;
    QLineEdit *nameLineEdit;
    QLineEdit *emailLineEdit;
};

class RegisterPage : public LicenseWizardPage
{
public:
    RegisterPage(LicenseWizard *wizard);

    void resetPage();
    WizardPage *nextPage();
    bool isComplete();

private:
    QLabel *topLabel;
    QLabel *nameLabel;
    QLabel *upgradeKeyLabel;
    QLabel *bottomLabel;
    QLineEdit *nameLineEdit;
    QLineEdit *upgradeKeyLineEdit;
};

class DetailsPage : public LicenseWizardPage
{
public:
    DetailsPage(LicenseWizard *wizard);

    void resetPage();
    WizardPage *nextPage();
    bool isComplete();

private:
    QLabel *topLabel;
    QLabel *companyLabel;
    QLabel *emailLabel;
    QLabel *postalLabel;
    QLineEdit *companyLineEdit;
    QLineEdit *emailLineEdit;
    QLineEdit *postalLineEdit;
};

class FinishPage : public LicenseWizardPage
{
public:
    FinishPage(LicenseWizard *wizard);

    void resetPage();
    bool isLastPage() { return true; }
    bool isComplete();

private:
    QLabel *topLabel;
    QLabel *bottomLabel;
    QCheckBox *agreeCheckBox;
};

#endif
