#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qptrlist.h>

#include <qbig5codec.h>
#include <private/qfontcodecs_p.h>


class TWTextCodecs : public QTextCodecPlugin
{
public:
    TWTextCodecs();
    
    QStringList keys() const;
    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );

private:
    QPtrList<QTextCodec> codecs;
};


TWTextCodecs::TWTextCodecs()
{
}

QStringList TWTextCodecs::keys() const
{
    QStringList list;
    list << "Big5" << "big5*-0";
    list << "MIB-2026" << "MIB--2026";
    return list;
}


QTextCodec *TWTextCodecs::createForMib( int mib )
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
	case -2026:
	    codec = new QFontBig5Codec;
	    break;

	case 2026:
	    codec = new QBig5Codec;
	    break;

	default:
	    ;
	}

	if (codec)
	    codecs.append(codec);
    }

    return codec;
}


QTextCodec *TWTextCodecs::createForName( const QString &name )
{
    QTextCodec *codec = 0;

    QPtrListIterator<QTextCodec> it(codecs);
    while ((codec = it.current())) {
	++it;

	if (codec->name() == name)
	    break;
    }

    if (! codec) {
	if (name == "Big5")
	    codec = new QBig5Codec;
	else if (name == "big5*-0")
	    codec = new QFontBig5Codec;

	if (codec)
	    codecs.append(codec);
    }

    return codec;
}


Q_EXPORT_PLUGIN( TWTextCodecs );

