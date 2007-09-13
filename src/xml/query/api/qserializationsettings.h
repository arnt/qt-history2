/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the $MODULE$ of the Qt Toolkit.
 * **
 * ** $TROLLTECH_DUAL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#ifndef QSERIALIZATIONSETTINGS_H
#define QSERIALIZATIONSETTINGS_H

#include <QtCore/QtGlobal>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Xml)

class QSerializationSettingsPrivate;
class QTextCodec;

class Q_XML_EXPORT QSerializationSettings
{
public:
    QSerializationSettings();
    QSerializationSettings(const QSerializationSettings &other);
    ~QSerializationSettings();

    void setCodec(QTextCodec *textCodec);
    QTextCodec *codec() const;

    void setIndentationEnabled(bool value);
    bool indentationEnabled() const;

private:
    QSerializationSettingsPrivate *d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
// vim: et:ts=4:sw=4:sts=4
