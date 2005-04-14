
#include <QtDesigner/QDesignerCustomWidgetInterface>

#include <Qt3Support/Q3Frame>
#include <QtCore/qplugin.h>

class Q3FramePlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    Q3FramePlugin(QObject *parent = 0);
    virtual ~Q3FramePlugin();

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
        <widget class=\"Q3Frame\" name=\"frame\">\
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

Q3FramePlugin::Q3FramePlugin(QObject *parent)
    : QObject(parent),
      m_initialized(false)
{
}

Q3FramePlugin::~Q3FramePlugin()
{
}


QString Q3FramePlugin::name() const
{
    return QLatin1String("Q3Frame");
}

QString Q3FramePlugin::group() const
{
    return QLatin1String("Compat");
}

QString Q3FramePlugin::toolTip() const
{
    return QString();
}

QString Q3FramePlugin::whatsThis() const
{
    return QString();
}

QString Q3FramePlugin::includeFile() const
{
    return QLatin1String("Qt3Support/Q3Frame");
}

QIcon Q3FramePlugin::icon() const
{
    return QIcon();
}

bool Q3FramePlugin::isContainer() const
{
    return true;
}

bool Q3FramePlugin::isForm() const
{
    return false;
}

QWidget *Q3FramePlugin::createWidget(QWidget *parent)
{
    return new Q3Frame(parent);
}

bool Q3FramePlugin::isInitialized() const
{
    return m_initialized;
}

void Q3FramePlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);
    m_initialized = true;
}


Q_EXPORT_PLUGIN(Q3FramePlugin)

#include "plugin.moc"
