#ifndef PROMOTETOCUSTOMWIDGETDIALOG_H
#define PROMOTETOCUSTOMWIDGETDIALOG_H

#include <QtCore/QPair>

#include "ui_promotetocustomwidgetdialog.h"

class AbstractWidgetDataBase;

class PromoteToCustomWidgetDialog : public QDialog,
                                    public Ui::PromoteToCustomWidgetDialog
{
    Q_OBJECT
public:
    PromoteToCustomWidgetDialog(AbstractWidgetDataBase *db,
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
    AbstractWidgetDataBase *m_db;
    typedef QPair<QString, QString> PromotedWidgetInfo;
    typedef QList<PromotedWidgetInfo> PromotedWidgetInfoList;
    PromotedWidgetInfoList m_promoted_list;
};

#endif // PROMOTETOCUSTOMWIDGETDIALOG_H
