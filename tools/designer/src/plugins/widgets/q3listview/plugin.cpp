
#include <container.h>
#include <customwidget.h>
#include <abstractformeditor.h>
#include <qextensionmanager.h>

#include <QObject>
#include <QIcon>
#include <Q3ListView>

#include <qplugin.h>


class Q3ListViewPlugin: public QObject, public ICustomWidget
{
    Q_OBJECT
    Q_INTERFACES(ICustomWidget)
public:
    inline Q3ListViewPlugin(QObject *parent = 0)
        : QObject(parent), m_initialized(false) {}
        
    virtual QString name() const
    { return QLatin1String("Q3ListView"); }
    
    virtual QString group() const
    { return QLatin1String("Compat"); }
    
    virtual QString toolTip() const
    { return QString::null; }
    
    virtual QString whatsThis() const
    { return QString::null; }
    
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
    
    virtual void initialize(AbstractFormEditor *core) 
    { 
        Q_UNUSED(core);
        
        if (m_initialized) 
            return;
            
        m_initialized = true;
    }
    
    virtual QString codeTemplate() const
    { return QString::null; }
    
private:
    bool m_initialized;
};

Q_EXPORT_PLUGIN(Q3ListViewPlugin)

#include "plugin.moc"
