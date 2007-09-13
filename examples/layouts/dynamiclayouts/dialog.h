/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QQueue>

QT_DECLARE_CLASS(QComboBox)
QT_DECLARE_CLASS(QDialogButtonBox)
QT_DECLARE_CLASS(QGridLayout)
QT_DECLARE_CLASS(QGroupBox)
QT_DECLARE_CLASS(QLabel)
QT_DECLARE_CLASS(QPushButton)

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

private slots:
    void buttonsOrientationChanged(int index);
    void rotateWidgets();
    void help();

private:
    void createRotableGroupBox();
    void createOptionsGroupBox();
    void createButtonBox();

    QGroupBox *rotableGroupBox;
    QQueue<QWidget *> rotableWidgets;

    QGroupBox *optionsGroupBox;
    QLabel *buttonsOrientationLabel;
    QComboBox *buttonsOrientationComboBox;

    QDialogButtonBox *buttonBox;
    QPushButton *closeButton;
    QPushButton *helpButton;
    QPushButton *rotateWidgetsButton;

    QGridLayout *mainLayout;
    QGridLayout *rotableLayout;
    QGridLayout *optionsLayout;
};

#endif
