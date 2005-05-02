
#include "q3groupbox_plugin.h"

#include <Qt3Support/Q3GroupBox>
#include <QtGui/QLayout>
#include <QtCore/qplugin.h>

Q3GroupBoxPlugin::Q3GroupBoxPlugin(QObject *parent)
    : QObject(parent),
      m_initialized(false)
{
}

Q3GroupBoxPlugin::~Q3GroupBoxPlugin()
{
}


QString Q3GroupBoxPlugin::name() const
{
    return QLatin1String("Q3GroupBox");
}

QString Q3GroupBoxPlugin::group() const
{
    return QLatin1String("Qt 3 Support");
}

QString Q3GroupBoxPlugin::toolTip() const
{
    return QString();
}

QString Q3GroupBoxPlugin::whatsThis() const
{
    return QString();
}

QString Q3GroupBoxPlugin::includeFile() const
{
    return QLatin1String("Qt3Support/Q3GroupBox");
}

QIcon Q3GroupBoxPlugin::icon() const
{
    return QIcon();
}

bool Q3GroupBoxPlugin::isContainer() const
{
    return true;
}

QWidget *Q3GroupBoxPlugin::createWidget(QWidget *parent)
{
    Q3GroupBox *g = new Q3GroupBox(parent);
    g->setColumnLayout(0, Qt::Vertical);
    g->layout()->setMargin(0);
    return g;
}

bool Q3GroupBoxPlugin::isInitialized() const
{
    return m_initialized;
}

void Q3GroupBoxPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);
    m_initialized = true;
}

