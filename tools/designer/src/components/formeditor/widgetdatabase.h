#ifndef WIDGETDATABASE_H
#define WIDGETDATABASE_H

#include "formeditor_global.h"

#include <abstractformeditor.h>
#include <abstractwidgetdatabase.h>

#include <QString>
#include <QIcon>

class QObject;

struct WidgetDataBaseItem: public AbstractWidgetDataBaseItem
{
    WidgetDataBaseItem(const QString &name = QString::null,
                       const QString &group = QString::null);

    QString name() const;
    void setName(const QString &name);

    QString group() const;
    void setGroup(const QString &group);

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    QString whatsThis() const;
    void setWhatsThis(const QString &whatsThis);

    QString includeFile() const;
    void setInludeFile(const QString &includeFile);

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    bool isContainer() const;
    void setContainer(bool b);

    bool isForm() const;
    void setForm(bool b);

    bool isCustom() const;
    void setCustom(bool b);

private:
    QString m_name;
    QString m_group;
    QString m_toolTip;
    QString m_whatsThis;
    QString m_includeFile;
    QIcon m_icon;
    uint m_container: 1;
    uint m_form: 1;
    uint m_custom: 1;
};

class QT_FORMEDITOR_EXPORT WidgetDataBase: public AbstractWidgetDataBase
{
    Q_OBJECT
public:
    WidgetDataBase(AbstractFormEditor *core, QObject *parent = 0);
    virtual ~WidgetDataBase();
    
    virtual AbstractFormEditor *core() const;    
    
    virtual AbstractWidgetDataBaseItem *item(int index) const;
    int indexOfObject(QObject *o, bool resolveName = true) const;
    
public slots:
    void loadPlugins();

private:
    AbstractFormEditor *m_core;
};

#endif // WIDGETDATABASE_H

