#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

class QString;
class QOpenType;
struct QCharAttributes;
struct QScriptItem;

class QScriptEngine
{
public:
    virtual void charAttributes( const QString &text, int from, int len, QCharAttributes *attributes );
    // shaped is an in/out paramter. It already contains the correct font engine
    virtual void shape( const QString &string, int from, int len, QScriptItem *item );

    // some helper methods that might get used by all script engines. Implemented in scriptenginebasic.cpp

    // try to position diacritics around it's base char in absence of any better way to determine
    // positioning (like open type tables)
    // needs a correct logClusters and glyphAttributes array.
    static void heuristicPosition( QScriptItem *item );

    // set the glyph attributes heuristically. Assumes a 1 to 1 relationship between chars ang glyphs
    // and no reordering (except for reversing if (bidiLevel % 2 ) )
    // also computes logClusters heuristically
    static void heuristicSetGlyphAttributes( const QString &text, int from, int len, QScriptItem *item );

    // internal
    static void calculateAdvances( QScriptItem *item );

};

// arabic and related

class QScriptEngineArabic : public QScriptEngine
{
public:
    void charAttributes( const QString &text, int from, int len, QCharAttributes *attributes );
    void shape( const QString &string, int from, int len, QScriptItem *item );
};

class QScriptEngineSyriac : public QScriptEngineArabic
{
public:
    void shape( const QString &string, int from, int len, QScriptItem *item );
};

// indic languages

class QScriptEngineBengali : public QScriptEngine
{
public:
    void shape( const QString &string, int from, int len, QScriptItem *item );
};

class QScriptEngineDevanagari : public QScriptEngine
{
public:
    void shape( const QString &string, int from, int len, QScriptItem *item );
};

class QScriptEngineTamil : public QScriptEngine
{
public:
    void shape( const QString &string, int from, int len, QScriptItem *item );
};

#endif
