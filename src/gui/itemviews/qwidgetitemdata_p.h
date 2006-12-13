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

#ifndef QWIDGETITEMDATA_P_H
#define QWIDGETITEMDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class QWidgetItemData
{
public:
    inline QWidgetItemData() : role(-1) {}
    inline QWidgetItemData(int r, QVariant v) : role(r), value(v) {}
    int role;
    QVariant value;
    inline bool operator==(const QWidgetItemData &other) { return role == other.role && value == other.value; }
};

#ifndef QT_NO_DATASTREAM

inline QDataStream &operator>>(QDataStream &in, QWidgetItemData &data)
{
    in >> data.role;
    in >> data.value;
    return in;
}

inline QDataStream &operator<<(QDataStream &out, const QWidgetItemData &data)
{
    out << data.role;
    out << data.value;
    return out;
}

#endif

#endif // QWIDGETITEMDATA_P_H
