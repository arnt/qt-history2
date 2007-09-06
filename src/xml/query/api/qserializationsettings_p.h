/****************************************************************************
 * **
 * ** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * ** * ** This file is part of the $MODULE$ of the Qt Toolkit.
 * **
 * ** $TROLLTECH_DUAL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QSerializationSettings_p_h
#define QSerializationSettings_p_h

QT_BEGIN_HEADER

class QSerializationSettingsPrivate
{
public:
    friend class QPreparedQuery;

    inline QSerializationSettingsPrivate() : codec(0)
                                           , indentationEnabled(true)
    {
    }

    QTextCodec  *codec;
    bool        indentationEnabled;
};

QT_END_HEADER
#endif
// vim: et:ts=4:sw=4:sts=4
