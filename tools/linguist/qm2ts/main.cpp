/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <metatranslator.h>

#include <qregexp.h>
#include <qstring.h>
#include <qtranslator.h>
#include <stdio.h>

typedef QList<TranslatorMessage> TML;

static void printUsage()
{
    fprintf( stderr, "Usage:\n"
             "    qm2ts [ options ] qm-files\n"
             "Options:\n"
             "    -help  Display this information and exit\n"
             "    -verbose\n"
             "           Explain what is being done\n"
             "    -version\n"
             "           Display the version of qm2ts and exit\n" );
}

int main( int argc, char **argv )
{
    bool verbose = false;
    int numQmFiles = 0;

    for ( int i = 1; i < argc; i++ ) {
        if ( qstrcmp(argv[i], "-help") == 0 ) {
            printUsage();
            return 0;
        } else if ( qstrcmp(argv[i], "-verbose") == 0 ) {
            verbose = true;
            continue;
        } else if ( qstrcmp(argv[i], "-version") == 0 ) {
            fprintf( stderr, "qm2ts version %s\n", QT_VERSION_STR );
            return 0;
        }

        numQmFiles++;
        Translator tor( 0 );
        if ( tor.load(argv[i], ".") ) {
            QString g = argv[i];
            g.replace( QRegExp(QString("\\.qm$")), QString() );
            g += QString( ".ts" );

            if ( verbose )
                fprintf( stderr, "Generating '%s'...\n", g.toLatin1().data() );

            MetaTranslator metator;
            int ignored = 0;

            TML all = tor.messages();
            TML::Iterator it;
            for ( it = all.begin(); it != all.end(); ++it ) {
                if ( (*it).sourceText() == 0 ) {
                    ignored++;
                } else {
                    QByteArray context = (*it).context();
                    if ( context.isEmpty() )
                        context = "@default";
                    metator.insert( MetaTranslatorMessage(context,
                                    (*it).sourceText(), (*it).comment(),
                                    (*it).translation(), false,
                                    MetaTranslatorMessage::Finished) );
                }
            }

            if ( !metator.save(g) ) {
                fprintf( stderr,
                         "qm2ts warning: For some reason, I cannot save '%s'\n",
                         g.toLatin1().constData() );
            } else {
                if ( verbose ) {
                    int converted = (int) metator.messages().count();
                    fprintf( stderr, " %d message%s converted (%d ignored)\n",
                             converted, converted == 1 ? "" : "s", ignored );
                }
                if ( ignored > 0 )
                    fprintf( stderr,
                             "qm2ts warning: File '%s' is not a Qt 2.x .qm"
                             " file (some information is lost)\n",
                             argv[i] );
            }
        } else {
            fprintf( stderr,
                     "qm2ts warning: For some reason, I cannot load '%s'\n",
                     argv[i] );
        }
    }

    if ( numQmFiles == 0 ) {
        printUsage();
        return 1;
    }
    return 0;
}
