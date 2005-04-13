
#include <QtDesigner/QDesignerCustomWidgetInterface>

#include <Qt3Support/Q3ButtonGroup>
#include <QtCore/qplugin.h>

class Q3ButtonGroupPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    Q3ButtonGroupPlugin(QObject *parent = 0);
    virtual ~Q3ButtonGroupPlugin();

    virtual QString name() const;
    virtual QString group() const;
    virtual QString toolTip() const;
    virtual QString whatsThis() const;
    virtual QString includeFile() const;
    virtual QIcon icon() const;

    virtual bool isContainer() const;
    virtual bool isForm() const;

    virtual QWidget *createWidget(QWidget *parent);

    virtual bool isInitialized() const;
    virtual void initialize(QDesignerFormEditorInterface *core);

    virtual QString domXml() const
    { return QLatin1String("\
        <widget class=\"Q3ButtonGroup\" name=\"Q3ButtonGroup\">\
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

Q3ButtonGroupPlugin::Q3ButtonGroupPlugin(QObject *parent)
    : QObject(parent),
      m_initialized(false)
{
}

Q3ButtonGroupPlugin::~Q3ButtonGroupPlugin()
{
}


QString Q3ButtonGroupPlugin::name() const
{
    return QLatin1String("Q3ButtonGroup");
}

QString Q3ButtonGroupPlugin::group() const
{
    return QLatin1String("Compat");
}

QString Q3ButtonGroupPlugin::toolTip() const
{
    return QString();
}

QString Q3ButtonGroupPlugin::whatsThis() const
{
    return QString();
}

QString Q3ButtonGroupPlugin::includeFile() const
{
    return QLatin1String("Qt3Support/Q3ButtonGroup");
}

QIcon Q3ButtonGroupPlugin::icon() const
{
    return QIcon();
}

bool Q3ButtonGroupPlugin::isContainer() const
{
    return true;
}

bool Q3ButtonGroupPlugin::isForm() const
{
    return false;
}

QWidget *Q3ButtonGroupPlugin::createWidget(QWidget *parent)
{
    Q3ButtonGroup *g = new Q3ButtonGroup(parent);
    g->setColumnLayout(0, Qt::Vertical);
    return g;
}

bool Q3ButtonGroupPlugin::isInitialized() const
{
    return m_initialized;
}

void Q3ButtonGroupPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);
    m_initialized = true;
}


Q_EXPORT_PLUGIN(Q3ButtonGroupPlugin)

#include "plugin.moc"
