#ifndef SCRIPTENGINEARABIC_H
#define SCRIPTENGINEARABIC_H

#include "scriptenginebasic.h"


class ScriptEngineArabic : public ScriptEngineBasic
{
public:
    virtual void charAttributes( const QString &text, int from, int len, CharAttributes *attributes );
    virtual void shape( ShapedItem *result );

};

#endif
