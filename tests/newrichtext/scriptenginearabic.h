#ifndef SCRIPTENGINEARABIC_H
#define SCRIPTENGINEARABIC_H

#include "scriptenginebasic.h"
#include "opentype.h"

class ScriptEngineArabic : public ScriptEngineBasic
{
public:
    void charAttributes( const QString &text, int from, int len, CharAttributes *attributes );
    void shape( ShapedItem *result );
    void position( ShapedItem *result );

protected:
    void openTypeShape( int script, const OpenTypeIface*, ShapedItem *result );
    void openTypePosition( int script, const OpenTypeIface *, ShapedItem *result );
};

#endif
