#ifndef SCRIPTENGINEARABIC_H
#define SCRIPTENGINEARABIC_H

#include "scriptenginebasic.h"
#include "opentype.h"

class QScriptEngineArabic : public QScriptEngineBasic
{
public:
    void charAttributes( const QString &text, int from, int len, QCharAttributes *attributes );
    void shape( QShapedItem *result );

protected:
    void openTypeShape( int script, const QOpenType*, QShapedItem *result );
};

#endif
