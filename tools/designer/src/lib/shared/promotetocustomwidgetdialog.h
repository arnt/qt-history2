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

#ifndef PROMOTETOCUSTOMWIDGETDIALOG_H
#define PROMOTETOCUSTOMWIDGETDIALOG_H

#include <QtCore/QPair>

#include "ui_promotetocustomwidgetdialog.h"

class QDesignerWidgetDataBaseInterface;

class PromoteToCustomWidgetDialog : public QDialog,
                                    public Ui::PromoteToCustomWidgetDialog
{
    Q_OBJECT
public:
    PromoteToCustomWidgetDialog(QDesignerWidgetDataBaseInterface *db,
                                const QString &base_class_name,
                                QWidget *parent = 0);
    virtual void accept();
    QString includeFile() const;
    QString customClassName() const;

private slots:
    void checkInputs();
    void setIncludeForClass(const QString &name);

private:
    bool m_automatic_include;
    QDesignerWidgetDataBaseInterface *m_db;
    typedef QPair<QString, QString> PromotedWidgetInfo;
    typedef QList<PromotedWidgetInfo> PromotedWidgetInfoList;
    PromotedWidgetInfoList m_promoted_list;
    QString m_base_class_name;
};

#endif // PROMOTETOCUSTOMWIDGETDIALOG_H
