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

#include <harfbuzz-shaper.h>
#include <QtCore/qglobal.h>

// temporary forward until all the textengine code has been moved to QtCore
Q_CORE_EXPORT void qGetCharAttributes(const HB_UChar16 *string, uint32_t stringLength,
                                      const HB_ScriptItem *items, uint32_t numItems,
                                      HB_CharAttributes *attributes);

Q_CORE_EXPORT HB_Bool qShapeItem(HB_ShaperItem *item);

// ### temporary
Q_CORE_EXPORT HB_Face qHBNewFace(void *font, HB_GetFontTableFunc tableFunc);
Q_CORE_EXPORT void qHBFreeFace(HB_Face);


