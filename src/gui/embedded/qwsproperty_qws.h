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

#ifndef QWSPROPERTY_QWS_H
#define QWSPROPERTY_QWS_H

#include "QtGui/qwscommand_qws.h"

QT_MODULE(Gui)

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

#ifndef QT_NO_QWS_PROPERTIES

class QWSPropertyManager
{
public:
    enum Mode {
        PropReplace = 0,
        PropPrepend,
        PropAppend
    };

    // pre-defined properties
    enum Atom {
        PropSelection = 0
    };

    QWSPropertyManager();
    ~QWSPropertyManager();

    bool setProperty(int winId, int property, int mode, const char *data, int len);
    bool hasProperty(int winId, int property);
    bool removeProperty(int winId, int property);
    bool addProperty(int winId, int property);
    bool getProperty(int winId, int property, char *&data, int &len);
    bool removeProperties(int winId);

private:
    class Data;
    Data* d;
};

#endif // QT_NO_QWS_PROPERTIES

#endif // QWSPROPERTY_QWS_H
