/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDESIGNER_PREFERENCES_H
#define QDESIGNER_PREFERENCES_H

#include "designer_enums.h"
#include <grid_p.h>
#include <previewconfigurationwidget_p.h>
#include <previewmanager_p.h>

#include <QtCore/QStringList>
#include <QtCore/QPoint>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>

QT_BEGIN_NAMESPACE

struct Preferences
{
    Preferences();

    UIMode m_uiMode;
    QFont m_font;
    QFontDatabase::WritingSystem m_writingSystem;
    bool m_useFont;
    QStringList m_additionalTemplatePaths;
    qdesigner_internal::Grid m_defaultGrid;
    qdesigner_internal::PreviewConfigurationWidgetState m_previewConfigurationWidgetState;
    qdesigner_internal::PreviewConfiguration m_previewConfiguration;
};

QT_END_NAMESPACE

#endif // QDESIGNER_PREFERENCES_H
