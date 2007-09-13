/****************************************************************************
 * ** * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.  * **
 * ** $TROLLTECH_GPL_LICENSE$
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

#ifndef Patternist_ColorOutput_h
#define Patternist_ColorOutput_h

#include <QtCore/QtGlobal>
#include <QtCore/QHash>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class ColorOutputPrivate;

class ColorOutput
{
    enum
    {
        ForegroundShift = 10,
        BackgroundShift = 20,
        SpecialShift    = 20,
        ForegroundMask  = ((1 << ForegroundShift) - 1) << ForegroundShift,
        BackgroundMask  = ((1 << BackgroundShift) - 1) << BackgroundShift
    };

public:
    enum ColorCodeComponent
    {
        BlackForeground         = 1 << ForegroundShift,
        BlueForeground          = 2 << ForegroundShift,
        GreenForeground         = 3 << ForegroundShift,
        CyanForeground          = 4 << ForegroundShift,
        RedForeground           = 5 << ForegroundShift,
        PurpleForeground        = 6 << ForegroundShift,
        BrownForeground         = 7 << ForegroundShift,
        LightGrayForeground     = 8 << ForegroundShift,
        DarkGrayForeground      = 9 << ForegroundShift,
        LightBlueForeground     = 10 << ForegroundShift,
        LightGreenForeground    = 11 << ForegroundShift,
        LightCyanForeground     = 12 << ForegroundShift,
        LightRedForeground      = 13 << ForegroundShift,
        LightPurpleForeground   = 14 << ForegroundShift,
        YellowForeground        = 15 << ForegroundShift,
        WhiteForeground         = 16 << ForegroundShift,

        BlackBackground         = 1 << BackgroundShift,
        BlueBackground          = 2 << BackgroundShift,
        GreenBackground         = 3 << BackgroundShift,
        CyanBackground          = 4 << BackgroundShift,
        RedBackground           = 5 << BackgroundShift,
        PurpleBackground        = 6 << BackgroundShift,
        BrownBackground         = 7 << BackgroundShift,
        DefaultColor            = 1 << SpecialShift
    };

    typedef QFlags<ColorCodeComponent> ColorCode;
    typedef QHash<int, ColorCode> ColorMapping;

    ColorOutput();
    ~ColorOutput();

    void setColorMapping(const ColorMapping &cMapping);
    ColorMapping colorMapping() const;
    void insertMapping(int colorID, const ColorCode colorCode);

    void writeUncolored(const QString &message);
    void write(const QString &message, int color = -1);
    QString colorify(const QString &message, int color = -1) const;

private:
    ColorOutputPrivate *d;
    Q_DISABLE_COPY(ColorOutput)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ColorOutput::ColorCode)

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
