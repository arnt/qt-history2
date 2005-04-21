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

#ifndef QDESIGNER_RESOURCE_H
#define QDESIGNER_RESOURCE_H

#include "formeditor_global.h"

#include <QtDesigner/abstractformbuilder.h>

#include <QtCore/QHash>
#include <QtCore/QStack>

class DomCustomWidgets;

class QLayoutWidget;
class QDesignerTabWidget;
class QDesignerStackedWidget;
class QDesignerToolBox;
class QDesignerContainerExtension;
class QDesignerWidgetDataBaseItemInterface;
class QDesignerFormEditorInterface;
class QDesignerCustomWidgetInterface;

namespace qdesigner { namespace components { namespace formeditor {

class FormWindow;

class QT_FORMEDITOR_EXPORT QDesignerResource : public QAbstractFormBuilder
{
public:
    QDesignerResource(FormWindow *fw);
    virtual ~QDesignerResource();

    virtual void save(QIODevice *dev, QWidget *widget);

    void copy(QIODevice *dev, const QList<QWidget*> &selection);
    DomUI *copy(const QList<QWidget*> &selection);
    QList<QWidget*> paste(DomUI *ui, QWidget *parentWidget);
    QList<QWidget*> paste(QIODevice *dev, QWidget *parentWidget);

    inline QDesignerFormEditorInterface *core() const
    { return m_core; }

protected:
    using QAbstractFormBuilder::create;
    using QAbstractFormBuilder::createDom;

    virtual void saveDom(DomUI *ui, QWidget *widget);
    virtual QWidget *create(DomUI *ui, QWidget *parentWidget);
    virtual QWidget *create(DomWidget *ui_widget, QWidget *parentWidget);
    virtual QLayout *create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget);
    virtual QLayoutItem *create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget);
    virtual void applyProperties(QObject *o, const QList<DomProperty*> &properties);
    virtual QList<DomProperty*> computeProperties(QObject *obj);
    virtual DomProperty *createProperty(QObject *object, const QString &propertyName, const QVariant &value);

    virtual QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name);
    virtual QLayout *createLayout(const QString &layoutName, QObject *parent, const QString &name);
    virtual void createCustomWidgets(DomCustomWidgets *);
    virtual void createAuthor(const QString&);
    virtual void createComment(const QString&);
    virtual void createResources(DomResources*);
    virtual void applyTabStops(QWidget *widget, DomTabStops *tabStops);

    virtual bool addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout);
    virtual bool addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget);

    virtual DomWidget *createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive = true);
    virtual DomLayout *createDom(QLayout *layout, DomLayout *ui_layout, DomWidget *ui_parentWidget);
    virtual DomLayoutItem *createDom(QLayoutItem *item, DomLayout *ui_layout, DomWidget *ui_parentWidget);

    virtual QAction *create(DomAction *ui_action, QObject *parent);
    virtual QActionGroup *create(DomActionGroup *ui_action_group, QObject *parent);
    virtual void addMenuAction(QAction *action);

    virtual DomAction *createDom(QAction *action);
    virtual DomActionGroup *createDom(QActionGroup *actionGroup);
    virtual DomActionRef *createActionRefDom(QAction *action);

    virtual QIcon nameToIcon(const QString &filePath, const QString &qrcPath);
    virtual QString iconToFilePath(const QIcon &pm) const;
    virtual QString iconToQrcPath(const QIcon &pm) const;
    virtual QPixmap nameToPixmap(const QString &filePath, const QString &qrcPath);
    virtual QString pixmapToFilePath(const QPixmap &pm) const;
    virtual QString pixmapToQrcPath(const QPixmap &pm) const;

    virtual bool checkProperty(QObject *obj, const QString &prop) const;

    bool checkProperty(QDesignerTabWidget *widget, const QString &prop) const;
    bool checkProperty(QDesignerStackedWidget *widget, const QString &prop) const;
    bool checkProperty(QDesignerToolBox *widget, const QString &prop) const;
    bool checkProperty(QLayoutWidget *widget, const QString &prop) const;

    DomWidget *saveWidget(QDesignerTabWidget *widget, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QDesignerStackedWidget *widget, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QDesignerToolBox *widget, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QWidget *widget, QDesignerContainerExtension *container, DomWidget *ui_parentWidget);

    virtual DomCustomWidgets *saveCustomWidgets();
    virtual DomTabStops *saveTabStops();
    virtual QString saveAuthor();
    virtual QString saveComment();
    virtual DomResources *saveResources();

    virtual void layoutInfo(DomLayout *layout, QObject *parent, int *margin, int *spacing);

    void changeObjectName(QObject *o, QString name);
    static QString qtify(const QString &name);

private:
    FormWindow *m_formWindow;
    bool m_isMainWidget;
    QDesignerFormEditorInterface *m_core;
    QHash<QString, QString> m_internal_to_qt;
    QHash<QString, QString> m_qt_to_internal;
    QHash<QString, QDesignerCustomWidgetInterface*> m_customFactory;
    QStack<QLayout*> m_chain;
    QHash<QDesignerWidgetDataBaseItemInterface*, bool> m_usedCustomWidgets;
    int m_topLevelSpacerCount;
    bool m_copyWidget;
};

} } } // namespace qdesigner::components::formeditor

#endif // QDESIGNER_RESOURCE_H
