/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.
 * **
 * ** $TROLLTECH_GPL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#ifndef QSerializationSettings_h
#define QSerializationSettings_h

#include <QtCore/QtGlobal>

QT_BEGIN_HEADER

QT_MODULE(Xml)

class QSerializationSettingsPrivate;
class QTextCodec;

class Q_DECL_EXPORT QSerializationSettings
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

QT_END_HEADER

#endif
// vim: et:ts=4:sw=4:sts=4
