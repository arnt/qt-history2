#ifndef SIMPLEWIZARD_H
#define SIMPLEWIZARD_H

#include <QDialog>
#include <QList>

class QHBoxLayout;
class QPushButton;
class QVBoxLayout;

class SimpleWizard : public QDialog
{
    Q_OBJECT

public:
    SimpleWizard(QWidget *parent = 0);

    void setButtonEnabled(bool enable);

protected:
    virtual QWidget *createPage(int index) = 0;
    void setNumPages(int n);

private slots:
    void previousButtonClicked();
    void nextButtonClicked();

private:
    void switchPage(QWidget *oldPage);

    QList<QWidget *> history;
    int numPages;
    QPushButton *cancelButton;
    QPushButton *previousButton;
    QPushButton *nextButton;
    QPushButton *finishButton;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
};

#endif
