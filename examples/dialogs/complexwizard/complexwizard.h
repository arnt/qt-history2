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

#ifndef COMPLEXWIZARD_H
#define COMPLEXWIZARD_H

#include <QDialog>
#include <QList>

class QHBoxLayout;
class QPushButton;
class QVBoxLayout;
class WizardPage;

class ComplexWizard : public QDialog
{
    Q_OBJECT

public:
    ComplexWizard(QWidget *parent = 0);

    QList<WizardPage *> historyPages() const { return history; }

protected:
    void setFirstPage(WizardPage *page);

private slots:
    void backButtonClicked();
    void nextButtonClicked();
    void completeStateChanged();

private:
    void switchPage(WizardPage *oldPage);

    QList<WizardPage *> history;
    QPushButton *cancelButton;
    QPushButton *backButton;
    QPushButton *nextButton;
    QPushButton *finishButton;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
};

class WizardPage : public QWidget
{
    Q_OBJECT

public:
    WizardPage(QWidget *parent = 0);

    virtual void resetPage();
    virtual WizardPage *nextPage();
    virtual bool isLastPage();
    virtual bool isComplete();

signals:
    void completeStateChanged();
};

#endif
