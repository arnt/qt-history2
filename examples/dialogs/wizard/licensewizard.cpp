#include <QtGui>

#include "licensewizard.h"

LicenseWizard::LicenseWizard(QWidget *parent)
    : Wizard(parent)
{
    titlePage = new TitlePage(this);
    evaluatePage = new EvaluatePage(this);
    registerPage = new RegisterPage(this);
    detailsPage = new DetailsPage(this);
    finishPage = new FinishPage(this);

    setFirstPage(titlePage);
}

TitlePage::TitlePage(LicenseWizard *wizard)
    : LicenseWizardPage(wizard)
{
    setBackgroundColor(Qt::red);

    lineEdit = new QLineEdit(this);
    lineEdit->setGeometry(20, 20, 100, 25);
    connect(lineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(completeStateChanged()));
}

void TitlePage::resetPage()
{
}

WizardPage *TitlePage::nextPage()
{
    return wizard()->registerPage;
}

bool TitlePage::isComplete()
{
    return !lineEdit->text().isEmpty();
}

EvaluatePage::EvaluatePage(LicenseWizard *wizard)
    : LicenseWizardPage(wizard)
{
    setBackgroundColor(Qt::green);
}

void EvaluatePage::resetPage()
{
}

WizardPage *EvaluatePage::nextPage()
{
    return wizard()->finishPage;
}

RegisterPage::RegisterPage(LicenseWizard *wizard)
    : LicenseWizardPage(wizard)
{
    setBackgroundColor(Qt::blue);
}

void RegisterPage::resetPage()
{
}

WizardPage *RegisterPage::nextPage()
{
    return wizard()->detailsPage;
}

DetailsPage::DetailsPage(LicenseWizard *wizard)
    : LicenseWizardPage(wizard)
{
    setBackgroundColor(Qt::cyan);
}

void DetailsPage::resetPage()
{
}

WizardPage *DetailsPage::nextPage()
{
    return wizard()->finishPage;
}

FinishPage::FinishPage(LicenseWizard *wizard)
    : LicenseWizardPage(wizard)
{
    setBackgroundColor(Qt::magenta);
}

void FinishPage::resetPage()
{
}
