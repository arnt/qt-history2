#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qptrlist.h>

#include <qeuckrcodec.h>
#include <private/qfontcodecs_p.h>


class KRTextCodecs : public QTextCodecPlugin
{
public:
    KRTextCodecs();

    QStringList keys() const;
    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );

private:
    QPtrList<QTextCodec> codecs;
};


KRTextCodecs::KRTextCodecs()
{
}


QStringList KRTextCodecs::keys() const
{
    QStringList list;
    list << "eucKR" << "ksc5601.1987-0";
    list << "MIB-38" << "MIB-36";
    return list;
}

QTextCodec *KRTextCodecs::createForMib( int mib )
{
    QTextCodec *codec = 0;

    QPtrListIterator<QTextCodec> it(codecs);
    while ((codec = it.current())) {
	++it;

	if (codec->mibEnum() == mib)
	    break;
    }

    if (! codec ) {
	switch (mib) {
	case 36:
	    codec = new QFontKsc5601Codec;
	    break;

	case 38:
	    codec = new QEucKrCodec;
	    break;

	default:
	    ;
	}

	if (codec)
	    codecs.append(codec);
    }

    return codec;
}


QTextCodec *KRTextCodecs::createForName( const QString &name )
{
    QTextCodec *codec = 0;

    QPtrListIterator<QTextCodec> it(codecs);
    while ((codec = it.current())) {
	++it;

	if (codec->name() == name)
	    break;
    }

    if (! codec) {
	if (name == "eucKR")
	    codec = new QEucKrCodec;
	else if (name == "ksc5601.1987-0")
	    codec = new QFontKsc5601Codec;

	if (codec)
	    codecs.append(codec);
    }

    return codec;
}


Q_EXPORT_PLUGIN( KRTextCodecs );
