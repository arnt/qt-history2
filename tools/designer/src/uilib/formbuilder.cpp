
#include "formbuilder.h"
#include <ui4.h>

#include <QtGui/QtGui>

FormBuilder::FormBuilder()
{
}

QWidget *FormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    QList<DomAction*> actions = ui_widget->elementAction();
    for (int i=0; i<actions.size(); ++i) {
        QAction *action = new QAction(/*widget*/);
        applyProperties(action, actions.at(i)->elementProperty());
        m_actions.insert(actions.at(i)->attributeName(), action);
    }

    if (QWidget *w = Resource::create(ui_widget, parentWidget)) {
        //if (QMenu *menu = qt_cast<QMenu*>(w)) {
            QList<DomActionRef*> refs = ui_widget->elementAddAction();
            for (int i=0; i<refs.size(); ++i) {
                if (QAction *a = m_actions.value(refs.at(i)->attributeName()))
                    w->addAction(a);
            }
        //}
        return w;
    }

    return 0;
}


QWidget *FormBuilder::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    QWidget *w = 0;

    if (qt_cast<QTabWidget*>(parentWidget)
            || qt_cast<QStackedWidget*>(parentWidget)
            || qt_cast<QToolBox*>(parentWidget))
        parentWidget = 0;

    if (widgetName == QLatin1String("Line"))
        w = new QFrame(parentWidget);
    else if (widgetName == QLatin1String("QToolBar"))
        w = new QToolBar(qt_cast<QMainWindow*>(parentWidget));

#define DECLARE_LAYOUT(L, C)
#define DECLARE_COMPAT_WIDGET(W, C) /*DECLARE_WIDGET(W, C)*/
#define DECLARE_WIDGET(W, C) else if (widgetName == QLatin1String(#W)) { Q_ASSERT(w == 0); w = new W(parentWidget); }

#include "widgets.table"

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET

    if (w) {
        w->setObjectName(name);
    } else {
        qWarning("widget `%s' not supported",
            widgetName.latin1());
    }

    if (qt_cast<QDialog *>(w))
        w->setParent(parentWidget, 0);
    return w;
}

QLayout *FormBuilder::createLayout(const QString &layoutName, QObject *parent, const QString &name)
{
    QLayout *l = 0;

    QWidget *parentWidget = qt_cast<QWidget*>(parent);
    QLayout *parentLayout = qt_cast<QLayout*>(parent);

    Q_ASSERT(parentWidget || parentLayout);

#define DECLARE_WIDGET(W, C)
#define DECLARE_COMPAT_WIDGET(W, C)
#define DECLARE_LAYOUT(L, C) \
    if (layoutName == QLatin1String(#L)) { \
        Q_ASSERT(l == 0); \
        l = parentLayout \
            ? new L(static_cast<QLayout*>(0)) \
            : new L(parentWidget); \
    }

#include "widgets.table"

#undef DECLARE_LAYOUT
#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_WIDGET

    if (l) {
        l->setObjectName(name);
    } else {
        qWarning("layout `%s' not supported",
            layoutName.latin1());
    }

    return l;
}

bool FormBuilder::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    return Resource::addItem(ui_item, item, layout);
}

bool FormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    return Resource::addItem(ui_widget, widget, parentWidget);
}

