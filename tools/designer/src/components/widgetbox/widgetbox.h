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

#ifndef WIDGETBOX_H
#define WIDGETBOX_H

#include <QtXml/QDomDocument>

#include <QtDesigner/abstractwidgetbox.h>
#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/abstractdnditem.h>

#include "widgetbox_global.h"

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QStackedLayout;
class QDomElement;
class QEvent;
class QMenu;
class QAction;
class QActionGroup;
class WidgetCollectionModel;
class Scratchpad;
class WidgetBoxTreeView;
class DomWidget;

class QT_WIDGETBOX_EXPORT WidgetBox : public QDesignerWidgetBoxInterface
{
    Q_OBJECT
public:
    WidgetBox(QDesignerFormEditorInterface *core, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~WidgetBox();

    QDesignerFormEditorInterface *core() const;

    virtual int categoryCount() const;
    virtual Category category(int cat_idx) const;
    virtual void addCategory(const Category &cat);
    virtual void removeCategory(int cat_idx);

    virtual int widgetCount(int cat_idx) const;
    virtual Widget widget(int cat_idx, int wgt_idx) const;
    virtual void addWidget(int cat_idx, const Widget &wgt);
    virtual void removeWidget(int cat_idx, int wgt_idx);

    void dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list, const QPoint &global_mouse_pos);

    virtual void setFileName(const QString &file_name);
    virtual QString fileName() const;
    virtual bool load();
    virtual bool save();

private slots:
    void handleMousePress(const QString &xml, const QPoint &global_mouse_pos);

private:
    QDesignerFormEditorInterface *m_core;
    WidgetBoxTreeView *m_view;
};

#endif // WIDGETBOX_H
