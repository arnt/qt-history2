/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef _SIGNALSLOTDIALOG_H
#define _SIGNALSLOTDIALOG_H

#include "shared_global_p.h"
#include <QtCore/QStringList>
#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QDesignerMemberSheet;
class QListView;
class QToolButton;
class QItemSelection;

namespace Ui {
    class SignalSlotDialogClass;
}

namespace qdesigner_internal {

// Dialog data
struct SignalSlotDialogData {
    void clear();
    QStringList m_existingMethods;
    QStringList m_fakeMethods;
};

// Internal helper class: A model for signatures that allows for verifying duplicates
// (checking signals versus slots and vice versa).
class SignatureModel : public QStandardItemModel {
    Q_OBJECT

public:
    SignatureModel(QObject *parent = 0);
    virtual bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

signals:
    void checkSignature(const QString &signature, bool *ok);
};

// Internal helper class: Panel for editing method signatures. List view with validator,
// add and remove button
class SignaturePanel  : public QObject {
     Q_OBJECT

public:
    SignaturePanel(QObject *parent, QListView *listView, QToolButton *addButton, QToolButton *removeButton, const QString &newPrefix);

    QStringList fakeMethods() const;
    void setData(const SignalSlotDialogData &d);
    int count(const QString &signature) const;

signals:
    void checkSignature(const QString &signature, bool *ok);

private slots:
    void slotAdd();
    void slotRemove();
    void slotSelectionChanged(const QItemSelection &, const QItemSelection &);

private:
    void closeEditor();

    const QString m_newPrefix;
    SignatureModel *m_model;
    QListView *m_listView;
    QToolButton *m_removeButton;
};

// Dialog for  editing signals and slots.
// Provides static convenience function
// to modify fake signals and slots. They are
// handled in 2 ways:
// 1) For the MainContainer: Fake signals and slots are stored
//    in the meta database (per-instance)
// 2) For promoted widgets: Fake signals and slots are stored
//    in the widget database (per-class)
// Arguably, we could require the MainContainer to be promoted for that, too,
// but that would require entering a header.

class QDESIGNER_SHARED_EXPORT SignalSlotDialog : public QDialog {
    Q_OBJECT

public:
    enum FocusMode { FocusSlots, FocusSignals };

    explicit SignalSlotDialog(QWidget *parent = 0, FocusMode m = FocusSlots);
    virtual ~SignalSlotDialog();

    DialogCode showDialog(SignalSlotDialogData &slotData, SignalSlotDialogData &signalData);

    // Edit fake methods stored in MetaDataBase (per instance, used for main containers)
    static bool editMetaDataBase(QDesignerFormWindowInterface *fw, QObject *object, QWidget *parent = 0, FocusMode m = FocusSlots);

    // Edit fake methods of a promoted class stored in WidgetDataBase (synthesizes a widget to obtain existing members).
    static bool editPromotedClass(QDesignerFormEditorInterface *core, const QString &promotedClassName, QWidget *parent = 0, FocusMode m = FocusSlots);
    // Edit fake methods of a promoted class stored in WidgetDataBase on a base class instance.
    static bool editPromotedClass(QDesignerFormEditorInterface *core, QObject *baseObject, QWidget *parent = 0, FocusMode m = FocusSlots);

private slots:
    void slotCheckSignature(const QString &signature, bool *ok);

private:
    // Edit fake methods of a promoted class stored in WidgetDataBase using an instance of the base class.
    static bool editPromotedClass(QDesignerFormEditorInterface *core, const QString &promotedClassName, QObject *baseObject, QWidget *parent, FocusMode m);

    const FocusMode m_focusMode;
    Ui::SignalSlotDialogClass *m_ui;
    SignaturePanel *m_slotPanel;
    SignaturePanel *m_signalPanel;
};
}
#endif
