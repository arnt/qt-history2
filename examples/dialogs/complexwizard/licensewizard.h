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

    TitlePage *titlePage;
    EvaluatePage *evaluatePage;
    RegisterPage *registerPage;
    DetailsPage *detailsPage;
    FinishPage *finishPage;
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

    QLabel *topLabel;
    QRadioButton *evaluateRadioButton;
    QRadioButton *registerRadioButton;
};

class EvaluatePage : public LicenseWizardPage
{
public:
    EvaluatePage(LicenseWizard *wizard);

    void resetPage();
    WizardPage *nextPage();
    bool isComplete();

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

    QLabel *topLabel;
    QLabel *bottomLabel;
    QCheckBox *agreeCheckBox;
};

#endif
