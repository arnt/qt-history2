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

#ifndef QXMLUTILS_P_H
#define QXMLUTILS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

class QString;

/*!
  \internal
  \short This class contains helper functions related to XML, for validating character classes,
         productions in the XML specification, and so on.

 This class is intentionally Not exported.
 */
class QXmlUtils
{
public:
    /*!
       Determines whether \a encName is a valid instance of production [81]EncName in the XML 1.0
       specification. If it is, true is returned, otherwise false.

        \sa \l {http://www.w3.org/TR/REC-xml/#NT-EncName}
               {Extensible Markup Language (XML) 1.0 (Fourth Edition), [81] EncName}
     */
    static bool isEncName(const QString &encName);
};

#endif
