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


#include <qsettings.h>
#include <qapplication.h>
#include <qmessagebox.h>

#include <qt_windows.h>
#include <ocidl.h>
#include <olectl.h>
#include "../shared/types.h"

class ListBoxText : public QListBoxText
{
public:
    ListBoxText(QListBox *box, const QString &name, const QString &id)
        : QListBoxText(box, name), ID(id)
    {
    }
    
    QString clsid() const
    {
        return ID;
    }
    
private:
    QString ID;
};

void QActiveXSelect::init()
{
    activex = 0;
    QApplication::setOverrideCursor(WaitCursor);
    HKEY classes_key;
    RegOpenKeyA(HKEY_CLASSES_ROOT, "CLSID", &classes_key);
    if (classes_key) {
        DWORD index = 0;
        LONG result = 0;
        char buffer[256];
        DWORD szBuffer = 255;
        FILETIME ft;
        do {
            result = RegEnumKeyExA(classes_key, index, (char*)&buffer, &szBuffer, 0, 0, 0, &ft);
            szBuffer = 255;
            if (result == ERROR_SUCCESS) {
                HKEY sub_key;
                QString clsid = QString::fromLocal8Bit(buffer);
                result = RegOpenKeyA(classes_key, QString(clsid + "\\Control").local8Bit(), &sub_key);
                if (result == ERROR_SUCCESS) {
                    RegCloseKey(sub_key);
                    RegQueryValueA(classes_key, buffer, (char*)&buffer, (LONG*)&szBuffer);
                    QString name = QString::fromLocal8Bit(buffer, szBuffer);
                    if (!name.isEmpty())
                        (void)new ListBoxText(ActiveXList, name, clsid);
                    
                }
                result = ERROR_SUCCESS;
            }
            szBuffer = 255;
            ++index;
        } while (result == ERROR_SUCCESS);
        RegCloseKey(classes_key);
    }
    
    ActiveXList->sort();
    QApplication::restoreOverrideCursor();
    
    ActiveXList->setFocus();
}


void QActiveXSelect::controlSelected(QListBoxItem *ctrl)
{
    if (!ctrl)
        return;
    
    ActiveX->setText(((ListBoxText*)ctrl)->clsid());
}

void QActiveXSelect::openLater()
{
    if (!activex || !activex->isNull() || !designer) {
        if (designer)
            designer->release();
        delete this;
        return;
    }
    if (exec()) {
        QUuid clsid = ActiveX->text();
        QString key;
        
        IClassFactory2 *cf2 = 0;
        CoGetClassObject(clsid, CLSCTX_SERVER, 0, IID_IClassFactory2, (void**)&cf2);
        if (cf2) {
            BSTR bKey;
            HRESULT hres = cf2->RequestLicKey(0, &bKey);
            if (hres == CLASS_E_NOTLICENSED) {
                QMessageBox::warning(parentWidget(), tr("Licensed Control"),
                    tr("The control requires a design-time license"));
                clsid = QUuid();
            } else {
                key = BSTRToQString(bKey);
            }
            cf2->Release();
        }
        
        if (!clsid.isNull()) {
            if (key.isEmpty())
                activex->setControl(clsid);
            else
                activex->setControl(clsid + ":" + key);
            
            DesignerFormWindow *form = designer->currentForm();
            if (form) {
                form->setPropertyChanged(activex, "control", true);
                form->clearSelection();
                qApp->processEvents();
                form->selectWidget(activex);
                form->setCurrentWidget(activex);
            }
        }
        designer->release();
        delete this;
    }
}

void QActiveXSelect::setActiveX(QAxWidget *ax)
{
    activex = ax;
}

void QActiveXSelect::setDesigner(DesignerInterface *des)
{
    designer = des;
    designer->addRef();
}


QString QActiveXSelect::selectedControl()
{
    return ActiveX->text();
}
