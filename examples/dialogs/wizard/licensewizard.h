#ifndef LICENSEWIZARD_H
#define LICENSEWIZARD_H

#include "wizard.h"

class TitlePage;
class EvaluatePage;
class RegisterPage;
class DetailsPage;
class FinishPage;

class LicenseWizard : public Wizard
{
    Q_OBJECT

public:
    LicenseWizard(QWidget *parent = 0);

private:
    TitlePage *titlePage;
    EvaluatePage *evaluatePage;
    RegisterPage *registerPage;
    DetailsPage *detailsPage;
    FinishPage *finishPage;

    friend class TitlePage;
    friend class EvaluatePage;
    friend class RegisterPage;
    friend class DetailsPage;
    friend class FinishPage;
};

class LicenseWizardPage : public WizardPage
{
public:
    LicenseWizardPage(LicenseWizard *wizard)
        : WizardPage(wizard), theWizard(wizard) {}

protected:
    LicenseWizard *wizard() { return theWizard; }

private:
    LicenseWizard *theWizard;
};

#include <qlineedit.h> // ###

class TitlePage : public LicenseWizardPage
{
public:
    TitlePage(LicenseWizard *wizard);

    void resetPage();
    WizardPage *nextPage();
    bool isComplete();

private:
    QLineEdit *lineEdit;
};

class EvaluatePage : public LicenseWizardPage
{
public:
    EvaluatePage(LicenseWizard *wizard);

    void resetPage();
    WizardPage *nextPage();
};

class RegisterPage : public LicenseWizardPage
{
public:
    RegisterPage(LicenseWizard *wizard);

    void resetPage();
    WizardPage *nextPage();
};

class DetailsPage : public LicenseWizardPage
{
public:
    DetailsPage(LicenseWizard *wizard);

    void resetPage();
    WizardPage *nextPage();
};

class FinishPage : public LicenseWizardPage
{
public:
    FinishPage(LicenseWizard *wizard);

    void resetPage();
    bool isLastPage() { return true; }
};

#endif
