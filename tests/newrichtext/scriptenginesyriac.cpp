#include "scriptenginesyriac.h"
#include "opentype.h"
#include "qfont.h"

void ScriptEngineSyriac::shape( ShapedItem *result )
{
    OpenTypeIface *openType = result->d->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( QFont::Syriac ) ) {
	openTypeShape( QFont::Syriac, openType, result );
	return;
    }
    ScriptEngineBasic::shape( result );
}
