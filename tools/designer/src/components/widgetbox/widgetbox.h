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
class WidgetBoxContainer;
class DomWidget;

class QT_WIDGETBOX_EXPORT WidgetBox : public AbstractWidgetBox
{
    Q_OBJECT
public:
    WidgetBox(AbstractFormEditor *core, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~WidgetBox();

    AbstractFormEditor *core() const;

    virtual int categoryCount() const;
    virtual Category category(int cat_idx) const;
    virtual void addCategory(const Category &cat);
    virtual void removeCategory(int cat_idx);

    virtual int widgetCount(int cat_idx) const;
    virtual Widget widget(int cat_idx, int wgt_idx) const;
    virtual void addWidget(int cat_idx, const Widget &wgt);
    virtual void removeWidget(int cat_idx, int wgt_idx);

public slots:
    virtual void reload();

private slots:
    void handleMousePress(const QString &xml, const QRect &geometry);

private:
    AbstractFormEditor *m_core;
    WidgetCollectionModel *m_model;
    WidgetBoxContainer *m_view;
};

#endif // WIDGETBOX_H
