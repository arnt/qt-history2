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

#include <buddyeditor/buddyeditor_plugin.h>
#include <signalsloteditor/signalsloteditor_plugin.h>
#include <tabordereditor/tabordereditor_plugin.h>

#include <QtCore/qplugin.h>

Q_IMPORT_PLUGIN(SignalSlotEditorPlugin)
Q_IMPORT_PLUGIN(BuddyEditorPlugin)
Q_IMPORT_PLUGIN(TabOrderEditorPlugin)

