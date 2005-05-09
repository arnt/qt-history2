/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LOCATIONDIALOG_H
#define LOCATIONDIALOG_H

#include <QDialog>
#include <QSettings>

class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QTableWidget;

class LocationDialog : public QDialog
{
    Q_OBJECT

public:
    LocationDialog(QWidget *parent = 0);

    QSettings::Format format() const;
    QSettings::Scope scope() const;
    QString organization() const;
    QString application() const;

private slots:
    void updateLocationsTable();

private:
    QLabel *formatLabel;
    QLabel *scopeLabel;
    QLabel *organizationLabel;
    QLabel *applicationLabel;
    QComboBox *formatComboBox;
    QComboBox *scopeComboBox;
    QComboBox *organizationComboBox;
    QComboBox *applicationComboBox;
    QGroupBox *locationsGroupBox;
    QTableWidget *locationsTable;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif
