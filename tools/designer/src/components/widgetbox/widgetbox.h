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

#include "widgetbox_global.h"
#include "abstractwidgetbox.h"
#include "abstractformwindowmanager.h"

#include <QDomDocument>

class AbstractFormEditor;
class AbstractFormWindow;
class QStackedLayout;
class QDomElement;
class QEvent;
class QMenu;
class QAction;
class QActionGroup;
class WidgetCollectionModel;
class Scratchpad;
class WidgetBoxView;
class DomWidget;

class QT_WIDGETBOX_EXPORT WidgetBox : public AbstractWidgetBox
{
    Q_OBJECT

public:
    enum ViewMode { TreeMode, FormMode };

    WidgetBox(AbstractFormEditor *core, ViewMode mode = TreeMode, QWidget *parent = 0,
              Qt::WFlags flags = 0);
    virtual ~WidgetBox();

    AbstractFormEditor *core() const;

    virtual int categoryCount() const;
    virtual DomUI *category(int cat_idx) const;
    virtual int widgetCount(int cat_idx) const;
    virtual DomUI *widget(int cat_idx, int wgt_idx) const;

    virtual int addCategory(const QString &name, const QString &icon_file, DomUI *ui);
    virtual void removeCategory(int cat_idx);

    inline ViewMode viewMode() const { return m_mode; }
    void setViewMode(ViewMode mode);

private slots:
    void handleMousePress(const QDomElement &elt, const QRect &geometry);
    void setViewMode(QAction *action);

private:
    AbstractFormEditor *m_core;
    WidgetCollectionModel *m_model;

    ViewMode m_mode;

    WidgetBoxView *m_view;

    QActionGroup *m_mode_action_group;
    QAction *m_tree_mode_action;
    QAction *m_form_mode_action;
};

#endif // WIDGETBOX_H
