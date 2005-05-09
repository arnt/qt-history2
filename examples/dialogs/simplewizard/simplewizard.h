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
    void backButtonClicked();
    void nextButtonClicked();

private:
    void switchPage(QWidget *oldPage);

    QList<QWidget *> history;
    int numPages;
    QPushButton *cancelButton;
    QPushButton *backButton;
    QPushButton *nextButton;
    QPushButton *finishButton;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
};

#endif
