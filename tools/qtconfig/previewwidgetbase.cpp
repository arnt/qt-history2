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

#include "previewwidgetbase.h"

#include <QVariant>

QT_BEGIN_NAMESPACE

/*
 *  Constructs a PreviewWidgetBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
PreviewWidgetBase::PreviewWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : QWidget(parent, name, fl)
{
    setupUi(this);


    // signals and slots connections
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
PreviewWidgetBase::~PreviewWidgetBase()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void PreviewWidgetBase::languageChange()
{
    retranslateUi(this);
}

void PreviewWidgetBase::init()
{
}

void PreviewWidgetBase::destroy()
{
}

QT_END_NAMESPACE
