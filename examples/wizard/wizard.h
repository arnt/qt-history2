/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WIZARD_H
#define WIZARD_H

#include <qwizard.h>

class QWidget;
class QHBox;
class QLineEdit;
class QLabel;

class Wizard : public QWizard
{
    Q_OBJECT

public:
    Wizard( QWidget *parent = 0, const char *name = 0 );

    void showPage(QWidget* page);

protected:
    void setupPage1();
    void setupPage2();
    void setupPage3();

    QHBox *page1, *page2, *page3;
    QLineEdit *key, *firstName, *lastName, *address, *phone, *email;
    QLabel *lKey, *lFirstName, *lLastName, *lAddress, *lPhone, *lEmail;

protected slots:
    void keyChanged( const QString & );
    void dataChanged( const QString & );

};

#endif
