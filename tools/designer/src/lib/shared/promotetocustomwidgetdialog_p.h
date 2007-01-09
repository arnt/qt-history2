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

#ifndef PROMOTETOCUSTOMWIDGETDIALOG_H
#define PROMOTETOCUSTOMWIDGETDIALOG_H

#include <QtGui/QDialog>
#include <QtCore/QHash>
#include <QtCore/QList>

class QDesignerWidgetDataBaseInterface;
class QDesignerWidgetDataBaseItemInterface;

namespace qdesigner_internal {

namespace Ui {
    class PromoteToCustomWidgetDialog;
} // namespace Ui

class PromoteToCustomWidgetDialog : public QDialog
{
    Q_OBJECT
public:
    typedef  QList<QDesignerWidgetDataBaseItemInterface*> WidgetDataBaseItemList;
    
    PromoteToCustomWidgetDialog(QDesignerWidgetDataBaseInterface *db,
                                const WidgetDataBaseItemList &candidates,
                                const QString &base_class_name,
                                QWidget *parent = 0);
    virtual ~PromoteToCustomWidgetDialog();

    virtual void accept();
    QString includeFile() const;
    QString customClassName() const;
    bool isGlobalInclude() const;

private slots:
    void checkInputs();

private:
    void setIncludeForClass(const QString &name);

    void warn(const QString &caption, const QString &what);
    bool ask(const QString &caption, const QString &what);
    const QString m_base_class_name;
    
    Ui::PromoteToCustomWidgetDialog *m_ui;

    const QDesignerWidgetDataBaseInterface *m_db;
    // Include file and global flag
    typedef QPair<QString, bool> PromotedWidgetInfo;
    // Class name to header
    typedef QHash<QString, PromotedWidgetInfo> PromotedWidgetInfoHash;
    PromotedWidgetInfoHash m_promotedHash;
};

} // namespace qdesigner_internal

#endif // PROMOTETOCUSTOMWIDGETDIALOG_H
