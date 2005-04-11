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

#include <qplugin.h>

#include <QtCore/QObject>
#include <QIcon>
#include <Q3ListView>


class Q3ListViewPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    inline Q3ListViewPlugin(QObject *parent = 0)
        : QObject(parent), m_initialized(false) {}
        
    virtual QString name() const
    { return QLatin1String("Q3ListView"); }
    
    virtual QString group() const
    { return QLatin1String("Compat"); }
    
    virtual QString toolTip() const
    { return QString(); }
    
    virtual QString whatsThis() const
    { return QString(); }
    
    virtual QString includeFile() const
    { return QLatin1String("q3listview.h"); }
    
    virtual QIcon icon() const
    { return QIcon(); }

    virtual bool isContainer() const
    { return false; }
    
    virtual bool isForm() const
    { return false; }

    virtual QWidget *createWidget(QWidget *parent)
    { return new Q3ListView(parent); }
    
    virtual bool isInitialized() const 
    { return m_initialized; }
    
    virtual void initialize(QDesignerFormEditorInterface *core) 
    { 
        Q_UNUSED(core);
        
        if (m_initialized) 
            return;
            
        m_initialized = true;
    }
    
    virtual QString codeTemplate() const
    { return QString(); }

    virtual QString domXml() const
    { return QLatin1String("\
        <widget class=\"Q3ListView\" name=\"Q3ListView\">\
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

Q_EXPORT_PLUGIN(Q3ListViewPlugin)

#include "plugin.moc"
