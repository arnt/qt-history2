#include "scriptengine.h"


class ScriptEngineLatin : public ScriptEngine
{
public:
    virtual void charAttributes( const QString &text, int from, int len, CharAttributes *attributes );
    virtual void shape( const FontEngineIface &f, const QString &text, int from, int len,
			const ScriptAnalysis &analysis, ShapedItem *result );

    virtual int cursorToX( int cursorPos, const FontEngineIface &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped );
    virtual int xToCursor( int x, const FontEngineIface &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped );
};
