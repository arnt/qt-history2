#ifndef WIZARD_H
#define WIZARD_H

#include <QDialog>

class QHBoxLayout;
class QPushButton;
class QVBoxLayout;

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

class Wizard : public QDialog
{
    Q_OBJECT

public:
    Wizard(QWidget *parent = 0);

    void setFirstPage(WizardPage *page);

private slots:
    void previousButtonClicked();
    void nextButtonClicked();
    void completeStateChanged();

private:
    void switchPage(WizardPage *oldPage);

    QList<WizardPage *> history;
    QPushButton *cancelButton;
    QPushButton *previousButton;
    QPushButton *nextButton;
    QPushButton *finishButton;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
};

#endif
