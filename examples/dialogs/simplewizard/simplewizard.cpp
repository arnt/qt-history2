#include <QtGui>

#include "simplewizard.h"

SimpleWizard::SimpleWizard(QWidget *parent)
    : QDialog(parent)
{
    cancelButton = new QPushButton(tr("Cancel"));
    backButton = new QPushButton(tr("< &Back"));
    nextButton = new QPushButton(tr("Next >"));
    finishButton = new QPushButton(tr("&Finish"));

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

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

void SimpleWizard::setButtonEnabled(bool enable)
{
    if (history.size() == numPages)
        finishButton->setEnabled(enable);
    else
        nextButton->setEnabled(enable);
}

void SimpleWizard::setNumPages(int n)
{
    numPages = n;
    history.append(createPage(0));
    switchPage(0);
}

void SimpleWizard::backButtonClicked()
{
    nextButton->setEnabled(true);
    finishButton->setEnabled(true);

    QWidget *oldPage = history.takeLast();
    switchPage(oldPage);
    delete oldPage;
}

void SimpleWizard::nextButtonClicked()
{
    nextButton->setEnabled(true);
    finishButton->setEnabled(history.size() == numPages - 1);

    QWidget *oldPage = history.last();
    history.append(createPage(history.size()));
    switchPage(oldPage);
}

void SimpleWizard::switchPage(QWidget *oldPage)
{
    if (oldPage) {
        oldPage->hide();
        mainLayout->removeWidget(oldPage);
    }

    QWidget *newPage = history.last();
    mainLayout->insertWidget(0, newPage);
    newPage->show();
    newPage->setFocus();

    backButton->setEnabled(history.size() != 1);
    if (history.size() == numPages) {
        nextButton->setEnabled(false);
        finishButton->setDefault(true);
    } else {
        nextButton->setDefault(true);
        finishButton->setEnabled(false);
    }

    setWindowTitle(tr("Simple Wizard - Step %1 of %2")
                   .arg(history.size())
                   .arg(numPages));
}
