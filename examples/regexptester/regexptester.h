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

#ifndef REGEXPTESTER_H
#define REGEXPTESTER_H

#include <qdialog.h>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QStatusBar;
class QTable;

class RegexpTester : public QDialog
{
    Q_OBJECT

public:
    RegexpTester(QWidget* parent=0, const char* name=0, bool modal=false,
		 WFlags f=0);

    QLabel *regexLabel;
    QComboBox *regexComboBox;
    QLabel *textLabel;
    QComboBox *textComboBox;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *minimalCheckBox;
    QCheckBox *wildcardCheckBox;
    QTable *resultTable;
    QPushButton *executePushButton;
    QPushButton *copyPushButton;
    QPushButton *quitPushButton;
    QStatusBar *statusBar;

public slots:
    void copy();
    void execute();

private:
    void languageChange();
};

#endif // REGEXPTESTER_H
