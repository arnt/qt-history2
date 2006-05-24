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

#ifndef ORDERDIALOG_P_H
#define ORDERDIALOG_P_H

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

#include <QDialog>
#include "ui_orderdialog.h"

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class OrderDialog: public QDialog
{
    Q_OBJECT
public:
    OrderDialog(QDesignerFormWindowInterface *form, QWidget *parent);
    virtual ~OrderDialog();

    void setPageList(QList<QWidget*> *pages);

private slots:
    void on_upButton_clicked();
    void on_downButton_clicked();
    void on_okButton_clicked();
    void on_pageList_currentRowChanged(int row);

private:
    Ui::OrderDialog ui;
    QDesignerFormWindowInterface *m_form;
    QList<QWidget*> *m_pages;
};

}  // namespace qdesigner_internal

#endif // ORDERDIALOG_P_H
