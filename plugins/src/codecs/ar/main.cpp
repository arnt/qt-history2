#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qptrlist.h>

#include <private/qfontcodecs_p.h>


class ARTextCodecs : public QTextCodecPlugin
{
public:
    ARTextCodecs();

    QStringList keys() const;
    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );

private:
    QPtrList<QTextCodec> codecs;
};


ARTextCodecs::ARTextCodecs()
{
}

QStringList ARTextCodecs::keys() const
{
    QStringList list;
    list << "iso8859-6.8x";
    return list;
}


QTextCodec *ARTextCodecs::createForMib( int )
{
    return 0;
}

QTextCodec *ARTextCodecs::createForName( const QString &name )
{
    QTextCodec *codec = 0;

    QPtrListIterator<QTextCodec> it(codecs);
    while ((codec = it.current())) {
	++it;

	if (codec->name() == name)
	    break;
    }

    if (! codec) {
	if (name == "iso8859-6.8x")
	    codec = new QFontArabic68Codec;

	if (codec)
	    codecs.append(codec);
    }

    return codec;
}


Q_EXPORT_PLUGIN( ARTextCodecs )
