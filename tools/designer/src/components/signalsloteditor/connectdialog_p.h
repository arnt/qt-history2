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

#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "ui_connectdialog.h"
#include <QtGui/QDialog>

class QDesignerFormWindowInterface;
class QPushButton;

namespace qdesigner_internal {

class ConnectDialog : public QDialog
{
    Q_OBJECT
public:
    ConnectDialog(QDesignerFormWindowInterface *formWindow, QWidget *sender, QWidget *receiver, QWidget *parent = 0);

    QString signal() const;
    QString slot() const;

    void setSignalSlot(const QString &signal, const QString &slot);

    bool showAllSignalsSlots() const;
    void setShowAllSignalsSlots(bool showIt);

private slots:
    void populateLists();
    void selectSignal(QListWidgetItem *item);
    void selectSlot(QListWidgetItem *item);
    void populateSignalList();
    void populateSlotList(const QString &signal = QString());
    void editSignals();
    void editSlots();

private:
    enum WidgetMode { NormalWidget, MainContainer, PromotedWidget };

    static WidgetMode widgetMode(QWidget *w,  QDesignerFormWindowInterface *formWindow);
    QPushButton *okButton();
    void setOkButtonEnabled(bool);
    void editSignalsSlots(QWidget *w, WidgetMode mode, int signalSlotDialogMode);

    QWidget *m_source;
    QWidget *m_destination;
    const WidgetMode m_sourceMode;
    const WidgetMode m_destinationMode;
    QDesignerFormWindowInterface *m_formWindow;
    Ui::ConnectDialog m_ui;
};

}
#endif // CONNECTDIALOG_H
