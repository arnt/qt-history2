#ifndef RESOURCE_H
#define RESOURCE_H

#include "uilib_global.h"

#include <QList>
#include <QHash>
#include <QPalette>
#include <QSizePolicy>

class QObject;
class QVariant;
class QWidget;
class QLayout;
class QLayoutItem;
class QSpacerItem;
class QIODevice;
class QAction;
class QActionGroup;

class DomUI;
class DomWidget;
class DomLayout;
class DomLayoutItem;
class DomProperty;
class DomColorGroup;
class DomSpacer;
class DomString;
class DomTabStops;
class DomConnections;
class DomCustomWidgets;
class DomAction;
class DomActionGroup;
class DomActionRef;

class QT_UILIB_EXPORT Resource
{
public:
    Resource();
    virtual ~Resource();

    virtual QWidget *load(QIODevice *dev, QWidget *parentWidget=0);
    virtual void save(QIODevice *dev, QWidget *widget);

protected:
//
// load
//
    virtual QWidget *create(DomUI *ui, QWidget *parentWidget);
    virtual QWidget *create(DomWidget *ui_widget, QWidget *parentWidget);
    virtual QLayout *create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget);
    virtual QLayoutItem *create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget);
    
    virtual QAction *create(DomAction *ui_action, QObject *parent);
    virtual QActionGroup *create(DomActionGroup *ui_action_group, QObject *parent);

    virtual void applyProperties(QObject *o, const QList<DomProperty*> &properties);
    virtual void applyTabStops(QWidget *widget, DomTabStops *tabStops);

    virtual QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name);
    virtual QLayout *createLayout(const QString &layoutName, QObject *parent, const QString &name);
    virtual QAction *createAction(QObject *parent, const QString &name);
    virtual QActionGroup *createActionGroup(QObject *parent, const QString &name);
    
    virtual void createConnections(DomConnections *, QWidget *) {}

    virtual bool addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout);
    virtual bool addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget);

//
// save
//
    virtual DomWidget *createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive = true);
    virtual DomLayout *createDom(QLayout *layout, DomLayout *ui_layout, DomWidget *ui_parentWidget);
    virtual DomLayoutItem *createDom(QLayoutItem *item, DomLayout *ui_layout, DomWidget *ui_parentWidget);
    virtual DomSpacer *createDom(QSpacerItem *spacer, DomLayout *ui_layout, DomWidget *ui_parentWidget);
    virtual DomConnections *saveConnections();
    virtual DomCustomWidgets *saveCustomWidgets();
    virtual QList<DomProperty*> computeProperties(QObject *obj);
    virtual bool checkProperty(QObject *obj, const QString &prop) const;

    virtual void layoutInfo(DomWidget *widget, QObject *parent, int *margin, int *spacing);
    virtual void layoutInfo(DomLayout *layout, QObject *parent, int *margin, int *spacing);

//
// utils
//
    QVariant toVariant(const QMetaObject *meta, DomProperty *property);
    static bool toBool(const QString &str);
    static QString toString(const DomString *str);
    static QHash<QString, DomProperty*> propertyMap(const QList<DomProperty*> &properties);

    static bool isVertical(const QString &str);

    QHash<QObject*, bool> m_laidout;
    QHash<QString, QAction*> m_actions;
    QHash<QString, QActionGroup*> m_actionGroups;
    
private:
    void setupColorGroup(QPalette &palette, DomColorGroup *group);
    DomColorGroup *saveColorGroup(const QPalette &palette);
    
    QHash<QString, QSizePolicy::SizeType> m_idToSizeType;
    int m_defaultMargin;
    int m_defaultSpacing;

private:
    Resource(const Resource &other);
    void operator = (const Resource &other);
};

#endif

