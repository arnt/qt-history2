#include <QtGui>

#include "complexwizard.h"

ComplexWizard::ComplexWizard(QWidget *parent)
    : QDialog(parent)
{
    cancelButton = new QPushButton(tr("Cancel"), this);
    backButton = new QPushButton(tr("< &Back"), this);
    nextButton = new QPushButton(tr("Next >"), this);
    finishButton = new QPushButton(tr("&Finish"), this);

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(backButton, SIGNAL(clicked()), this, SLOT(backButtonClicked()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
    connect(finishButton, SIGNAL(clicked()), this, SLOT(accept()));

    buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(nextButton);
    buttonLayout->addWidget(finishButton);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(buttonLayout);
}

void ComplexWizard::setFirstPage(WizardPage *page)
{
    page->resetPage();
    history.append(page);
    switchPage(0);
}

void ComplexWizard::backButtonClicked()
{
    WizardPage *oldPage = history.takeLast();
    oldPage->resetPage();
    switchPage(oldPage);
}

void ComplexWizard::nextButtonClicked()
{
    WizardPage *oldPage = history.last();
    WizardPage *newPage = oldPage->nextPage();
    newPage->resetPage();
    history.append(newPage);
    switchPage(oldPage);
}

void ComplexWizard::completeStateChanged()
{
    WizardPage *currentPage = history.last();
    if (currentPage->isLastPage())
        finishButton->setEnabled(currentPage->isComplete());
    else
        nextButton->setEnabled(currentPage->isComplete());
}

void ComplexWizard::switchPage(WizardPage *oldPage)
{
    if (oldPage) {
        oldPage->hide();
        mainLayout->removeWidget(oldPage);
        disconnect(oldPage, SIGNAL(completeStateChanged()),
                   this, SLOT(completeStateChanged()));
    }

    WizardPage *newPage = history.last();
    mainLayout->insertWidget(0, newPage);
    newPage->show();
    newPage->setFocus();
    connect(newPage, SIGNAL(completeStateChanged()),
            this, SLOT(completeStateChanged()));

    backButton->setEnabled(history.size() != 1);
    if (newPage->isLastPage()) {
        nextButton->setEnabled(false);
        finishButton->setDefault(true);
    } else {
        nextButton->setDefault(true);
        finishButton->setEnabled(false);
    }
    completeStateChanged();
}

WizardPage::WizardPage(QWidget *parent)
    : QWidget(parent)
{
    hide();
}

void WizardPage::resetPage()
{
}

WizardPage *WizardPage::nextPage()
{
    return 0;
}

bool WizardPage::isLastPage()
{
    return false;
}

bool WizardPage::isComplete()
{
    return true;
}
