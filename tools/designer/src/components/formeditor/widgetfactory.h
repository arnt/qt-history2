#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include "formeditor_global.h"
#include "abstractwidgetfactory.h"

#include <pluginmanager.h>

#include <QMap>
#include <QVariant>
#include <QPointer>

class QObject;
class QWidget;
class QLayout;
class AbstractFormEditor;
struct ICustomWidget;

class QT_FORMEDITOR_EXPORT WidgetFactory: public AbstractWidgetFactory
{
public:
    WidgetFactory(AbstractFormEditor *core, QObject *parent = 0);
    ~WidgetFactory();

    virtual QWidget* containerOfWidget(QWidget *widget) const;
    virtual QWidget* widgetOfContainer(QWidget *widget) const;
    
    virtual QWidget *createWidget(const QString &className, QWidget *parentWidget) const;
    virtual QLayout *createLayout(QWidget *widget, QLayout *layout, int type) const;
    virtual void initialize(QObject *object) const;

    virtual AbstractFormEditor *core() const;

    static const char* classNameOf(QObject* o);
    
public slots:
    void loadPlugins();
    
private:
    AbstractFormEditor *m_core;
    QMap<QString, ICustomWidget*> m_customFactory;
};

#endif
