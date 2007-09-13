/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

QT_DECLARE_CLASS(QDataWidgetMapper)
QT_DECLARE_CLASS(QLabel)
QT_DECLARE_CLASS(QLineEdit)
QT_DECLARE_CLASS(QPushButton)
QT_DECLARE_CLASS(QSpinBox)
QT_DECLARE_CLASS(QStandardItemModel)
QT_DECLARE_CLASS(QTextEdit)

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = 0);

private slots:
    void updateButtons(int row);

private:
    void setupModel();

    QLabel *nameLabel;
    QLabel *addressLabel;
    QLabel *ageLabel;
    QLineEdit *nameEdit;
    QTextEdit *addressEdit;
    QSpinBox *ageSpinBox;
    QPushButton *nextButton;
    QPushButton *previousButton;

    QStandardItemModel *model;
    QDataWidgetMapper *mapper;
};

#endif
