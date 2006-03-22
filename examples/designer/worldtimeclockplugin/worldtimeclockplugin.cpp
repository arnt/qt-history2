/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "worldtimeclock.h"
#include "worldtimeclockplugin.h"

#include <QtPlugin>

WorldTimeClockPlugin::WorldTimeClockPlugin(QObject *parent)
    : QObject(parent)
{
    initialized = false;
}

void WorldTimeClockPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool WorldTimeClockPlugin::isInitialized() const
{
    return initialized;
}

QWidget *WorldTimeClockPlugin::createWidget(QWidget *parent)
{
    return new WorldTimeClock(parent);
}

QString WorldTimeClockPlugin::name() const
{
    return "WorldTimeClock";
}

QString WorldTimeClockPlugin::group() const
{
    return "Display Widgets [Examples]";
}

QIcon WorldTimeClockPlugin::icon() const
{
    return QIcon();
}

QString WorldTimeClockPlugin::toolTip() const
{
    return "";
}

QString WorldTimeClockPlugin::whatsThis() const
{
    return "";
}

bool WorldTimeClockPlugin::isContainer() const
{
    return false;
}

QString WorldTimeClockPlugin::domXml() const
{
    return "<widget class=\"WorldTimeClock\" name=\"worldTimeClock\">\n"
           " <property name=\"geometry\">\n"
           "  <rect>\n"
           "   <x>0</x>\n"
           "   <y>0</y>\n"
           "   <width>100</width>\n"
           "   <height>100</height>\n"
           "  </rect>\n"
           " </property>\n"
           "</widget>\n";
}

QString WorldTimeClockPlugin::includeFile() const
{
    return "worldtimeclock.h";
}

Q_EXPORT_PLUGIN2(worldtimeclockplugin, WorldTimeClockPlugin)
