#include "scriptengine.h"


class ScriptEngineArabic : public ScriptEngine
{
public:
    virtual void charAttributes( const QString &text, int from, int len, CharAttributes *attributes );
    virtual void shape( const QString &text, int from, int len, ShapedItem *result );

    virtual int cursorToX( int cursorPos, const QString &text, int from, int len, const ShapedItem &shaped );
    virtual int xToCursor( int x, const QString &text, int from, int len, const ShapedItem &shaped );
};
