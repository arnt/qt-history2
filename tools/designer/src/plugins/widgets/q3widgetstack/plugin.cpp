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

#include <QtDesigner/container.h>
#include <QtDesigner/customwidget.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/default_extensionfactory.h>

#include <QtCore/QObject>
#include <QIcon>
#include <Q3WidgetStack>

#include <qplugin.h>
#include <QtCore/qdebug.h>

class Q3WidgetStackContainer: public QObject, public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    inline Q3WidgetStackContainer(Q3WidgetStack *widget, QObject *parent = 0)
        : QObject(parent), 
          m_widget(widget) {}
        
    virtual int count() const
    { return m_pages.count(); }
    
    virtual QWidget *widget(int index) const
    { 
        if (index == -1)
            return 0;
            
        return m_pages.at(index); 
    }

    virtual int currentIndex() const
    { return m_pages.indexOf(m_widget->visibleWidget()); }
    
    virtual void setCurrentIndex(int index)
    { m_widget->raiseWidget(m_pages.at(index)); }

    virtual void addWidget(QWidget *widget)
    {
        m_pages.append(widget);
        m_widget->addWidget(widget);
    }
    
    virtual void insertWidget(int index, QWidget *widget)
    {
        m_pages.insert(index, widget);
        m_widget->addWidget(widget);
    }
    
    virtual void remove(int index)
    {
        m_widget->removeWidget(m_pages.at(index));
        m_pages.removeAt(index);
    }

private:
    Q3WidgetStack *m_widget;
    QList<QWidget*> m_pages;
};

class Q3WidgetStackContainerFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    inline Q3WidgetStackContainerFactory(QExtensionManager *parent = 0)
        : QExtensionFactory(parent) {}

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const
    {
        if (iid != Q_TYPEID(QDesignerContainerExtension))
            return 0;
            
        if (Q3WidgetStack *w = qobject_cast<Q3WidgetStack*>(object))
            return new Q3WidgetStackContainer(w, parent);
        
        return 0;
    }
};

class Q3WidgetStackPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    inline Q3WidgetStackPlugin(QObject *parent = 0)
        : QObject(parent), m_initialized(false) {}
        
    virtual QString name() const
    { return QLatin1String("Q3WidgetStack"); }
    
    virtual QString group() const
    { return QLatin1String("Compat"); }
    
    virtual QString toolTip() const
    { return QString(); }
    
    virtual QString whatsThis() const
    { return QString(); }
    
    virtual QString includeFile() const
    { return QLatin1String("q3widgetstack.h"); }
    
    virtual QIcon icon() const
    { return QIcon(); }

    virtual bool isContainer() const
    { return true; }
    
    virtual bool isForm() const
    { return false; }

    virtual QWidget *createWidget(QWidget *parent)
    { return new Q3WidgetStack(parent); }
    
    virtual bool isInitialized() const 
    { return m_initialized; }
    
    virtual void initialize(QDesignerFormEditorInterface *core) 
    { 
        Q_UNUSED(core);
        
        if (m_initialized) 
            return;
            
        m_initialized = true;
        QExtensionManager *mgr = core->extensionManager();
        mgr->registerExtensions(new Q3WidgetStackContainerFactory(mgr), Q_TYPEID(QDesignerContainerExtension));
    }
    
    virtual QString codeTemplate() const
    { return QString(); }
    
    virtual QString domXml() const
    { return QLatin1String("\
        <widget class=\"Q3WidgetStack\" name=\"Q3WidgetStack\">\
            <property name=\"geometry\">\
                <rect>\
                    <x>0</x>\
                    <y>0</y>\
                    <width>100</width>\
                    <height>80</height>\
                </rect>\
            </property>\
        </widget>\
      "); }

private:
    bool m_initialized;
};

Q_EXPORT_PLUGIN(Q3WidgetStackPlugin)

#include "plugin.moc"
