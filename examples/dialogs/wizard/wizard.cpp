#include <QtGui>

#include "wizard.h"

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

Wizard::Wizard(QWidget *parent)
    : QDialog(parent)
{
    cancelButton = new QPushButton(tr("Cancel"), this);
    previousButton = new QPushButton(tr("< Previous"), this);
    nextButton = new QPushButton(tr("Next >"), this);
    finishButton = new QPushButton(tr("Finish"), this);

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(previousButton, SIGNAL(clicked()), this, SLOT(previousButtonClicked()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
    connect(finishButton, SIGNAL(clicked()), this, SLOT(accept()));

    buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(previousButton);
    buttonLayout->addWidget(nextButton);
    buttonLayout->addWidget(finishButton);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(buttonLayout);
}

void Wizard::setFirstPage(WizardPage *page)
{
    page->resetPage();
    history.append(page);
    switchPage(0);
}

void Wizard::previousButtonClicked()
{
    WizardPage *oldPage = history.takeLast();
    oldPage->resetPage();
    switchPage(oldPage);
}

void Wizard::nextButtonClicked()
{
    WizardPage *oldPage = history.last();
    WizardPage *newPage = oldPage->nextPage();
    newPage->resetPage();
    history.append(newPage);
    switchPage(oldPage);
}

void Wizard::completeStateChanged()
{
    WizardPage *currentPage = history.last();
    if (currentPage->isLastPage()) {
        finishButton->setEnabled(currentPage->isComplete());
    } else {
        nextButton->setEnabled(currentPage->isComplete());
    }
}

void Wizard::switchPage(WizardPage *oldPage)
{
    if (oldPage) {
        mainLayout->removeWidget(oldPage);
        oldPage->hide();
        disconnect(oldPage, SIGNAL(completeStateChanged()), this, SLOT(completeStateChanged()));
    }

    WizardPage *newPage = history.last();
    mainLayout->insertWidget(0, newPage);
    newPage->show();
    connect(newPage, SIGNAL(completeStateChanged()), this, SLOT(completeStateChanged()));

    previousButton->setDisabled(history.size() == 1);

    bool isLastPage = newPage->isLastPage();
    nextButton->setDisabled(isLastPage);
    finishButton->setDisabled(!isLastPage);
    if (isLastPage)
        finishButton->setDefault(true);
    else
        nextButton->setDefault(true);
    completeStateChanged();
}
