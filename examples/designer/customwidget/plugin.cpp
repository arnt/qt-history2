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

#include "analogclock.h"
#include "plugin.h"

AnalogClockPlugin::AnalogClockPlugin(QObject *parent)
    : QObject(parent)
{
    initialized = false;
}

void AnalogClockPlugin::initialize(AbstractFormEditor * /*core*/) 
{ 
    if (initialized) 
        return;

    initialized = true;
}

bool AnalogClockPlugin::isInitialized() const 
{
    return initialized;
}

QWidget *AnalogClockPlugin::createWidget(QWidget *parent)
{
    return new AnalogClock(parent);
}

QString AnalogClockPlugin::name() const
{
    return QLatin1String("AnalogClock");
}

QString AnalogClockPlugin::group() const
{
    return QLatin1String("Display Widgets");
}

QIcon AnalogClockPlugin::icon() const
{
    return QIcon();
}

QString AnalogClockPlugin::toolTip() const
{
    return QString();
}

QString AnalogClockPlugin::whatsThis() const
{
    return QString();
}

bool AnalogClockPlugin::isContainer() const
{
    return false;
}

bool AnalogClockPlugin::isForm() const
{
    return false;
}

QString AnalogClockPlugin::domXml() const
{
    return QLatin1String("<widget class=\"AnalogClock\" name=\"AnalogClock\">\n"
                         " <property name=\"geometry\">\n"
                         "  <rect>\n"
                         "   <x>0</x>\n"
                         "   <y>0</y>\n"
                         "   <width>100</width>\n"
                         "   <height>100</height>\n"
                         "  </rect>\n"
                         " </property>\n"
                         "</widget>\n");
}

QString AnalogClockPlugin::includeFile() const
{
    return QLatin1String("analogclock.h");
}

QString AnalogClockPlugin::codeTemplate() const
{
    return QString();
}

Q_EXPORT_PLUGIN(AnalogClockPlugin)
