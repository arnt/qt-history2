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

#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include "propertyeditor_global.h"
#include <qdesigner_propertyeditor_p.h>

#include <QtCore/QPointer>
#include <QtCore/QMap>

class DomProperty;
class QDesignerMetaDataBaseItemInterface;
class QDesignerPropertySheetExtension;

class QtAbstractPropertyBrowser;
class QtGroupBoxPropertyBrowser;
class QtTreePropertyBrowser;
class QtProperty;
class QtVariantProperty;

class QStackedWidget;
class QLabel;
class QMenu;
class QSignalMapper;

namespace qdesigner_internal {

class StringProperty;
class DesignerPropertyManager;

class QT_PROPERTYEDITOR_EXPORT PropertyEditor: public QDesignerPropertyEditor
{
    Q_OBJECT
public:
    explicit PropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~PropertyEditor();

    virtual QDesignerFormEditorInterface *core() const;

    virtual bool isReadOnly() const;
    virtual void setReadOnly(bool readOnly);
    virtual void setPropertyValue(const QString &name, const QVariant &value, bool changed = true);
    virtual void setPropertyComment(const QString &name, const QString &value);
    virtual void updatePropertySheet();

    virtual void setObject(QObject *object);

    virtual QObject *object() const
    { return m_object; }

    virtual QString currentPropertyName() const;

private slots:
    void slotResetProperty(QtProperty *property);
    void slotValueChanged(QtProperty *property, const QVariant &value);
    void slotViewTriggered(QAction *action);
    void slotAddDynamicProperty();

private:
    void updateBrowserValue(QtVariantProperty *property, const QVariant &value);
    int toBrowserType(const QVariant &value, const QString &propertyName) const;
    QString removeScope(const QString &value) const;
    QDesignerMetaDataBaseItemInterface *metaDataBaseItem() const;
    void setupStringProperty(QtVariantProperty *property, const QString &pname, const QVariant &value, bool isMainContainer);
    void setupPaletteProperty(QtVariantProperty *property);
    QString realClassName(QObject *object) const;

    QDesignerFormEditorInterface *m_core;
    QDesignerPropertySheetExtension *m_propertySheet;
    QtAbstractPropertyBrowser *m_currentBrowser;
    QtGroupBoxPropertyBrowser *m_groupBrowser;
    QtTreePropertyBrowser *m_treeBrowser;
    DesignerPropertyManager *m_propertyManager;
    QPointer<QObject> m_object;
    QMap<QString, QtVariantProperty*> m_nameToProperty;
    QMap<QtProperty*, QString> m_propertyToGroup;
    QMap<QString, QtVariantProperty*> m_nameToGroup;
    QMap<QtVariantProperty *, QtVariantProperty *> m_propertyToComment;
    QMap<QtVariantProperty *, QtVariantProperty *> m_commentToProperty;
    QList<QtProperty *> m_groups;
    bool m_updatingBrowser;
    QSignalMapper *m_removeMapper;

    QStackedWidget *m_stackedWidget;
    int m_groupBoxIndex;
    int m_treeIndex;
    QAction *m_addDynamicAction;
    QAction *m_removeDynamicAction;
    QMenu *m_removeDynamicMenu;
    QAction *m_treeAction;
    QAction *m_groupBoxAction;
    QLabel *m_classLabel;
};

}  // namespace qdesigner_internal

#endif // PROPERTYEDITOR_H
