#include "scriptenginesyriac.h"
#include "opentype.h"
#include "qfont.h"

void QScriptEngineSyriac::shape( QShapedItem *result )
{
    QOpenType *openType = result->d->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( QFont::Syriac ) ) {
	openTypeShape( QFont::Syriac, openType, result );
	return;
    }
    QScriptEngineBasic::shape( result );
}
