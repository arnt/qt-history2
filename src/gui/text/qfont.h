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

#ifndef QFONT_H
#define QFONT_H

#include "qwindowdefs.h"
#include "qstring.h"


class QFontPrivate;                                     /* don't touch */
class QStringList;
class Q3TextFormatCollection;

class Q_GUI_EXPORT QFont
{
public:
    enum StyleHint {
        Helvetica,  SansSerif = Helvetica,
        Times,      Serif = Times,
        Courier,    TypeWriter = Courier,
        OldEnglish, Decorative = OldEnglish,
        System,
        AnyStyle
    };

    enum StyleStrategy {
        PreferDefault    = 0x0001,
        PreferBitmap     = 0x0002,
        PreferDevice     = 0x0004,
        PreferOutline    = 0x0008,
        ForceOutline     = 0x0010,
        PreferMatch      = 0x0020,
        PreferQuality    = 0x0040,
        PreferAntialias  = 0x0080,
        NoAntialias      = 0x0100,
        OpenGLCompatible = 0x0200
    };

    enum Weight {
        Light    = 25,
        Normal   = 50,
        DemiBold = 63,
        Bold     = 75,
        Black         = 87
    };

    enum Stretch {
        UltraCondensed =  50,
        ExtraCondensed =  62,
        Condensed      =  75,
        SemiCondensed  =  87,
        Unstretched    = 100,
        SemiExpanded   = 112,
        Expanded       = 125,
        ExtraExpanded  = 150,
        UltraExpanded  = 200
    };

    QFont();
    QFont(const QString &family, int pointSize = -1, int weight = -1, bool italic = false);
    QFont(const QFont &, QPaintDevice *pd);
    QFont(const QFont &);
    ~QFont();

    QString family() const;
    void setFamily(const QString &);

    int pointSize() const;
    float pointSizeFloat() const;
    void setPointSize(int);
    void setPointSizeFloat(float);

    int pixelSize() const;
    void setPixelSize(int);
    void setPixelSizeFloat(float);

    int weight() const;
    void setWeight(int);

    bool bold() const;
    void setBold(bool);

    bool italic() const;
    void setItalic(bool);

    bool underline() const;
    void setUnderline(bool);

    bool overline() const;
    void setOverline(bool);

    bool strikeOut() const;
    void setStrikeOut(bool);

    bool fixedPitch() const;
    void setFixedPitch(bool);

    bool kerning() const;
    void setKerning(bool);

    StyleHint styleHint() const;
    StyleStrategy styleStrategy() const;
    void setStyleHint(StyleHint, StyleStrategy = PreferDefault);
    void setStyleStrategy(StyleStrategy s);

    int stretch() const;
    void setStretch(int);

    // is raw mode still needed?
    bool rawMode() const;
    void setRawMode(bool);

    // dupicated from QFontInfo
    bool exactMatch() const;

    QFont &operator=(const QFont &);
    bool operator==(const QFont &) const;
    bool operator!=(const QFont &) const;
    bool operator<(const QFont &) const;
    bool isCopyOf(const QFont &) const;


#ifdef Q_WS_WIN
    HFONT handle() const;
#else // !Q_WS_WIN
    Qt::HANDLE handle() const;
#endif // Q_WS_WIN


    // needed for X11
    void setRawName(const QString &);
    QString rawName() const;

    QString key() const;

    QString toString() const;
    bool fromString(const QString &);

    static QString substitute(const QString &);
    static QStringList substitutes(const QString &);
    static QStringList substitutions();
    static void insertSubstitution(const QString&, const QString &);
    static void insertSubstitutions(const QString&, const QStringList &);
    static void removeSubstitution(const QString &);
    static void initialize();
    static void cleanup();
#ifndef Q_WS_QWS
    static void cacheStatistics();
#endif

    // a copy of this lives in qunicodetables.cpp, as we can't include
    // qfont.h it in tools/. Do not modify without changing the script
    // enum in qunicodetable_p.h aswell.
    enum Script {
        // European Alphabetic Scripts
        Latin,
        Greek,
        Cyrillic,
        Armenian,
        Georgian,
        Runic,
        Ogham,
        SpacingModifiers,
        CombiningMarks,

        // Middle Eastern Scripts
        Hebrew,
        Arabic,
        Syriac,
        Thaana,

        // South and Southeast Asian Scripts
        Devanagari,
        Bengali,
        Gurmukhi,
        Gujarati,
        Oriya,
        Tamil,
        Telugu,
        Kannada,
        Malayalam,
        Sinhala,
        Thai,
        Lao,
        Tibetan,
        Myanmar,
        Khmer,

        // East Asian Scripts
        Han,
        Hiragana,
        Katakana,
        Hangul,
        Bopomofo,
        Yi,

        // Additional Scripts
        Ethiopic,
        Cherokee,
        CanadianAboriginal,
        Mongolian,

        // Symbols
        CurrencySymbols,
        LetterlikeSymbols,
        NumberForms,
        MathematicalOperators,
        TechnicalSymbols,
        GeometricSymbols,
        MiscellaneousSymbols,
        EnclosedAndSquare,
        Braille,

        Unicode,

        // some scripts added in Unicode 3.2
        Tagalog,
        Hanunoo,
        Buhid,
        Tagbanwa,

        KatakanaHalfWidth,

        // from Unicode 4.0
        Limbu,
        TaiLe,

        // End
        NScripts,
        UnknownScript = NScripts,

        NoScript,

        // ----------------------------------------
        // Dear User, you can see values > NScript,
        // but they are internal - do not touch.

        Han_Japanese,
        Han_SimplifiedChinese,
        Han_TraditionalChinese,
        Han_Korean,

        LastPrivateScript
    };

    QString defaultFamily() const;
    QString lastResortFamily() const;
    QString lastResortFont() const;

#ifdef QT_COMPAT
    static QT_COMPAT QFont defaultFont();
    static QT_COMPAT void setDefaultFont(const QFont &);
#endif // QT_COMPAT

    QFont resolve(const QFont &) const;
    inline uint resolve() const { return resolve_mask; }
    inline void resolve(uint mask) { resolve_mask = mask; }

protected:
    // why protected?
    bool dirty() const;
    int deciPointSize() const;

private:
    QFont(QFontPrivate *);

    void detach();

#if defined(Q_WS_MAC)
    void macSetFont(QPaintDevice *);
    friend class QQuickDrawPaintEngine;
#elif defined(Q_WS_X11)
    void x11SetScreen(int screen = -1);
    int x11Screen() const;
#endif

    friend class QFontMetrics;
    friend class QFontInfo;
    friend class QPainter;
    friend class QPSPrintEngineFont;
    friend class QApplication;
    friend class QWidget;
    friend class QWidgetPrivate;
    friend class Q3TextFormatCollection;
    friend class QTextLayout;
    friend class QTextEngine;
    friend class QTextLine;
    friend struct QScriptLine;
    friend class QGLContext;
    friend class QWin32PaintEngine;
    friend class QPainterPath;

#ifndef QT_NO_DATASTREAM
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QFont &);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QFont &);
#endif

    QFontPrivate *d;
    uint resolve_mask;
};


inline bool QFont::bold() const
{ return weight() > Normal; }


inline void QFont::setBold(bool enable)
{ setWeight(enable ? Bold : Normal); }




/*****************************************************************************
  QFont stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QFont &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QFont &);
#endif


#endif // QFONT_H
