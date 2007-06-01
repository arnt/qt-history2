#include "lrelease.h"

void LRelease::releaseMetaTranslator( const MetaTranslator& tor,
                                   const QString& qmFileName, bool verbose,
                                   bool ignoreUnfinished, bool trimmed )
{
    if ( verbose )
        Console::out(LRelease::tr( "Updating '%1'...\n").arg(qmFileName));
    if ( !tor.release(qmFileName, verbose, ignoreUnfinished,
                      trimmed ? Translator::Stripped
                               : Translator::Everything) )
        qWarning("lrelease warning: For some reason, I cannot save '%s'\n",
                 qmFileName.toLatin1().constData() );
}
