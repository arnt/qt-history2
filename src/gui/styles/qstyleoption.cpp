/****************************************************************************
**
** Implementation of the QStyleOption class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "qstyleoption.h"

Q4StyleOption::Q4StyleOption(int optionversion, int optiontype)
    : version(optionversion), type(optiontype)
{
}

void Q4StyleOption::init(const QWidget *w)
{
    state = 0;
    if (w->isEnabled())
        state |= QStyle::Style_Enabled;
    if (w->hasFocus())
        state |= QStyle::Style_HasFocus;
    rect = w->rect();
    palette = w->palette();
}

