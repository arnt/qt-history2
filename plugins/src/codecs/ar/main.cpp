#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qptrlist.h>

#include <private/qfontcodecs_p.h>


class ARTextCodecs : public QTextCodecPlugin
{
public:
    ARTextCodecs() {}

    QStringList names() const { return QStringList() << "iso8859-6.8x"; }
    QValueList<int> mibEnums() const { return QValueList<int>(); }
    QTextCodec *createForMib( int ) { return 0; }
    QTextCodec *createForName( const QString & );
};


QTextCodec *ARTextCodecs::createForName( const QString &name )
{
    if (name == "iso8859-6.8x")
	return new QFontArabic68Codec;

    return 0;
}


Q_EXPORT_PLUGIN( ARTextCodecs )
