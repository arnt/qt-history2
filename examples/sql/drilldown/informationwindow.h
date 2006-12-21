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

#ifndef INFORMATIONWINDOW_H
#define INFORMATIONWINDOW_H

#include <QtGui>
#include <QtSql>

class InformationWindow : public QDialog
{
    Q_OBJECT

public:
    InformationWindow(int id, QSqlRelationalTableModel *offices,
                      QWidget *parent = 0);

    int id();

signals:
    void imageChanged(int id, const QString &fileName);

private slots:
    void revert();
    void submit();
    void enableButtons(bool enable = true);

private:
    void createButtons();

    int locationId;
    QString storedImage;

    QComboBox *imageFileEditor;
    QLabel *locationText;
    QLabel *countryText;
    QTextEdit *descriptionEditor;

    QPushButton *closeButton;
    QPushButton *submitButton;
    QPushButton *revertButton;
    QDialogButtonBox *buttonBox;

    QDataWidgetMapper *mapper;
};

#endif
