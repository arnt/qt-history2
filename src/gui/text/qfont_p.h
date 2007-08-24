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

#ifndef QFONT_P_H
#define QFONT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qfont.h"
#include "QtCore/qmap.h"
#include "QtCore/qobject.h"
#include <private/qunicodetables_p.h>
#include <QtGui/qfontdatabase.h>

// forwards
class QFontEngine;

struct QFontDef
{
    inline QFontDef()
        : pointSize(-1.0), pixelSize(-1),
          styleStrategy(QFont::PreferDefault), styleHint(QFont::AnyStyle),
          weight(50), fixedPitch(false), style(QFont::StyleNormal), stretch(100),
          ignorePitch(true)
#ifdef Q_WS_MAC
          ,fixedPitchComputed(false)
#endif
    {
    }

    QString family;

#ifdef Q_WS_X11
    QString addStyle;
#endif // Q_WS_X11

    qreal pointSize;
    int pixelSize;

    uint styleStrategy : 16;
    uint styleHint     : 8;

    uint weight     :  7; // 0-99
    uint fixedPitch :  1;
    uint style      :  2;
    uint stretch    : 12; // 0-400

    uint ignorePitch : 1;
    uint fixedPitchComputed : 1; // for Mac OS X only
    int reserved   : 16; // for future extensions

    bool exactMatch(const QFontDef &other) const;
    bool operator==(const QFontDef &other) const
    {
        return pixelSize == other.pixelSize
                    && weight == other.weight
                    && style == other.style
                    && stretch == other.stretch
                    && styleHint == other.styleHint
                    && styleStrategy == other.styleStrategy
                    && family == other.family
#ifdef Q_WS_X11
                    && addStyle == other.addStyle
#endif
                          ;
    }
    inline bool operator<(const QFontDef &other) const
    {
        if (pixelSize != other.pixelSize) return pixelSize < other.pixelSize;
        if (weight != other.weight) return weight < other.weight;
        if (style != other.style) return style < other.style;
        if (stretch != other.stretch) return stretch < other.stretch;
        if (styleHint != other.styleHint) return styleHint < other.styleHint;
        if (styleStrategy != other.styleStrategy) return styleStrategy < other.styleStrategy;
        if (family != other.family) return family < other.family;

#ifdef Q_WS_X11
        if (addStyle != other.addStyle) return addStyle < other.addStyle;
#endif // Q_WS_X11

        return false;
    }
};

class QFontEngineData
{
public:
    QFontEngineData();
    ~QFontEngineData();

    QAtomic ref;

#if !defined(Q_WS_MAC)
    QFontEngine *engines[QUnicodeTables::ScriptCount];
#else
    QFontEngine *engine;
#endif
};


class Q_GUI_EXPORT QFontPrivate
{
public:
#ifdef Q_WS_X11
    static int defaultEncodingID;
#endif // Q_WS_X11

    QFontPrivate();
    QFontPrivate(const QFontPrivate &other);
    ~QFontPrivate();

#if !defined(Q_WS_MAC)
    inline QFontEngine *engineForScript(int script) const
    {
        if (script >= QUnicodeTables::Inherited)
            script = QUnicodeTables::Common;
        if (!engineData || !engineData->engines[script])
            QFontDatabase::load(this, script);
        return engineData->engines[script];
    }
#else
    inline QFontEngine *engineForScript(int script) const
    {
        if (script >= QUnicodeTables::Inherited)
            script = QUnicodeTables::Common;
        if (!engineData || !engineData->engine)
            QFontDatabase::load(this, script);
        return engineData->engine;
    }
#endif

    QAtomic ref;
    QFontDef request;
    mutable QFontEngineData *engineData;
    int dpi;
    int screen;

#ifdef Q_WS_WIN
    HDC hdc;
#endif

    uint rawMode    :  1;
    uint underline  :  1;
    uint overline   :  1;
    uint strikeOut  :  1;
    uint kerning    :  1;
    uint smallCaps  :  1;
    uint unused     :  2;
    
    short letterSpacing;
    short wordSpacing;

    enum {
        Family        = 0x0001,
        Size          = 0x0002,
        StyleHint     = 0x0004,
        StyleStrategy = 0x0008,
        Weight        = 0x0010,
        Style         = 0x0020,
        Underline     = 0x0040,
        Overline      = 0x0080,
        StrikeOut     = 0x0100,
        FixedPitch    = 0x0200,
        Stretch       = 0x0400,
        Kerning       = 0x0800,
        SmallCaps     = 0x1000,
        LetterSpacing = 0x2000,
        WordSpacing   = 0x4000,
        Complete      = 0x7fff
    };

    void resolve(uint mask, const QFontPrivate *other);
};


class QFontCache : public QObject
{
    Q_OBJECT
public:
    // note: these static functions work on a per-thread basis
    static QFontCache *instance();
    static void cleanup();

    QFontCache();
    ~QFontCache();

    void clear();
#if defined(Q_WS_QWS) && !defined(QT_NO_QWS_QPF2)
    void removeEngineForFont(const QByteArray &fontName);
#endif
    // universal key structure.  QFontEngineDatas and QFontEngines are cached using
    // the same keys
    struct Key {
        Key() : script(0), screen(0) { }
        Key(const QFontDef &d, int c, int s = 0)
            : def(d), script(c), screen(s) { }

        QFontDef def;
        int script;
        int screen;

        inline bool operator<(const Key &other) const
        {
            if (script != other.script) return script < other.script;
            if (screen != other.screen) return screen < other.screen;
            return def < other.def;
        }
        inline bool operator==(const Key &other) const
        { return def == other.def && script == other.script && screen == other.screen; }
    };

    // QFontEngineData cache
    typedef QMap<Key,QFontEngineData*> EngineDataCache;
    EngineDataCache engineDataCache;

    QFontEngineData *findEngineData(const Key &key) const;
    void insertEngineData(const Key &key, QFontEngineData *engineData);

    // QFontEngine cache
    struct Engine {
        Engine() : data(0), timestamp(0), hits(0) { }
        Engine(QFontEngine *d) : data(d), timestamp(0), hits(0) { }

        QFontEngine *data;
        uint timestamp;
        uint hits;
    };

    typedef QMap<Key,Engine> EngineCache;
    EngineCache engineCache;

    QFontEngine *findEngine(const Key &key);
    void insertEngine(const Key &key, QFontEngine *engine);

#if defined(Q_WS_WIN) || defined(Q_WS_QWS)
    void cleanupPrinterFonts();
#endif

    private:
    void increaseCost(uint cost);
    void decreaseCost(uint cost);
    void timerEvent(QTimerEvent *event);

    static const uint min_cost;
    uint total_cost, max_cost;
    uint current_timestamp;
    bool fast;
    int timer_id;
};

#endif // QFONT_P_H
