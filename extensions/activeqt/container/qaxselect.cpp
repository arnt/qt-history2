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

#include "qaxselect.h"

#include <qt_windows.h>

class ControlList : public QAbstractListModel
{
public:
    ControlList(QObject *parent=0)
    : QAbstractListModel(parent) 
    {
        HKEY classes_key;
        RegOpenKeyA(HKEY_CLASSES_ROOT, "CLSID", &classes_key);
        if (!classes_key)
            return;

        DWORD index = 0;
        LONG result = 0;
        char buffer[256];
        DWORD szBuffer = sizeof(buffer);
        FILETIME ft;
        do {
            result = RegEnumKeyExA(classes_key, index, (char*)&buffer, &szBuffer, 0, 0, 0, &ft);
            szBuffer = sizeof(buffer);
            if (result == ERROR_SUCCESS) {
                HKEY sub_key;
                QString clsid = QString::fromLocal8Bit(buffer);
                result = RegOpenKeyA(classes_key, QString(clsid + "\\Control").toLocal8Bit(), &sub_key);
                if (result == ERROR_SUCCESS) {
                    RegCloseKey(sub_key);
                    RegQueryValueA(classes_key, buffer, (char*)&buffer, (LONG*)&szBuffer);
                    QString name = QString::fromLocal8Bit(buffer, szBuffer);

                    controls << name;
                    clsids.insert(name, clsid);
                }
                result = ERROR_SUCCESS;
            }
            szBuffer = sizeof(buffer);
            ++index;
        } while (result == ERROR_SUCCESS);
        RegCloseKey(classes_key);

        controls.sort();
    }
    
    int rowCount(const QModelIndex & = QModelIndex()) const { return controls.count(); }
    QVariant data(const QModelIndex &index, int role) const;
    
private:
    QStringList controls;
    QMap<QString, QString> clsids;
};

QVariant ControlList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
        return controls.at(index.row());
    if (role == Qt::UserRole)
        return clsids.value(controls.at(index.row()));

    return QVariant();
}

QAxSelect::QAxSelect(QWidget *parent, Qt::WFlags f)
: QDialog(parent, f)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    setupUi(this);
    ActiveXList->setModel(new ControlList(this));
    connect(ActiveXList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
        this, SLOT(on_ActiveXList_clicked(const QModelIndex&)));
    QApplication::restoreOverrideCursor();
    ActiveXList->setFocus();

    connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void QAxSelect::on_ActiveXList_clicked(const QModelIndex &index)
{
    QVariant clsid = ActiveXList->model()->data(index, Qt::UserRole);
    ActiveX->setText(clsid.toString());
}

void QAxSelect::on_ActiveXList_doubleClicked(const QModelIndex &index)
{
    QVariant clsid = ActiveXList->model()->data(index, Qt::UserRole);
    ActiveX->setText(clsid.toString());

    accept();
}
