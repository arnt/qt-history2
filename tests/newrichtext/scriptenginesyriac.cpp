#include "scriptenginesyriac.h"
#include "opentype.h"

void ScriptEngineSyriac::shape( ShapedItem *result )
{
    OpenTypeIface *openType = result->d->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( OpenTypeIface::Syriac ) ) {
	openTypeShape( OpenTypeIface::Syriac, openType, result );
	return;
    }
    ScriptEngineBasic::shape( result );
}


void ScriptEngineSyriac::position( ShapedItem *result )
{
    OpenTypeIface *openType = result->d->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( OpenTypeIface::Syriac ) ) {
	openTypePosition( OpenTypeIface::Syriac, openType, result );
	return;
    }

    ScriptEngineBasic::position( result );
}
