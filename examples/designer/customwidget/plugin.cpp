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

#include <container.h>
#include <customwidget.h>
#include <abstractformeditor.h>
#include <qextensionmanager.h>

#include <qplugin.h>

#include <QIcon>
#include "analogclock.h"


class AnalogClockPlugin: public QObject, public ICustomWidget
{
    Q_OBJECT
    Q_INTERFACES(ICustomWidget)
public:
    inline AnalogClockPlugin(QObject *parent = 0)
        : QObject(parent), m_initialized(false) {}
        
    virtual QString name() const
    { return QLatin1String("AnalogClock"); }
    
    virtual QString group() const
    { return QLatin1String("Display"); }
    
    virtual QString toolTip() const
    { return QString(); }
    
    virtual QString whatsThis() const
    { return QString(); }
    
    virtual QString includeFile() const
    { return QLatin1String("analogclock.h"); }
    
    virtual QIcon icon() const
    { return QIcon(); }

    virtual bool isContainer() const
    { return false; }
    
    virtual bool isForm() const
    { return false; }

    virtual QWidget *createWidget(QWidget *parent)
    { return new AnalogClock(parent); }
    
    virtual bool isInitialized() const 
    { return m_initialized; }
    
    virtual void initialize(AbstractFormEditor *core) 
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
        <widget class=\"AnalogClock\" name=\"AnalogClock\">\
            <property name=\"geometry\">\
                <rect>\
                    <x>0</x>\
                    <y>0</y>\
                    <width>100</width>\
                    <height>100</height>\
                </rect>\
            </property>\
        </widget>\
      "); }
    
private:
    bool m_initialized;
};

Q_EXPORT_PLUGIN(AnalogClockPlugin)

#include "plugin.moc"
