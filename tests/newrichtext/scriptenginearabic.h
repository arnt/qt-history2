#ifndef SCRIPTENGINEARABIC_H
#define SCRIPTENGINEARABIC_H

#include "scriptenginebasic.h"


class ScriptEngineArabic : public ScriptEngineBasic
{
public:
    void charAttributes( const QString &text, int from, int len, CharAttributes *attributes );
    void shape( ShapedItem *result );
    void position( ShapedItem *result );

private:
    void openTypeShape( const OpenTypeIface*, ShapedItem *result );
    void openTypePosition( const OpenTypeIface *, ShapedItem *result );
};

#endif
