#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qptrlist.h>

#include <qeucjpcodec.h>
#include <qjiscodec.h>
#include <qsjiscodec.h>
#include <private/qfontcodecs_p.h>


class JPTextCodecs : public QTextCodecPlugin
{
public:
    JPTextCodecs();

    QStringList keys() const;
    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );

private:
    QPtrList<QTextCodec> codecs;
};


JPTextCodecs::JPTextCodecs()
{
}


QStringList JPTextCodecs::keys() const
{
    QStringList list;
    list << "eucJP" << "JIS7" << "SJIS" << "jisx0208.1983-0";
    list << "MIB-16" << "MIB-17" << "MIB-18" << "MIB-63";
    return list;
}


QTextCodec *JPTextCodecs::createForMib( int mib )
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
	case 16:
	    codec = new QJisCodec;
	    break;

	case 17:
	    codec = new QSjisCodec;
	    break;

	case 18:
	    codec = new QEucJpCodec;
	    break;

	case 63:
	    codec = new QFontJis0208Codec;
	    break;

	default:
	    ;
	}

	if (codec)
	    codecs.append(codec);
    }

    return codec;
}


QTextCodec *JPTextCodecs::createForName( const QString &name )
{
    QTextCodec *codec = 0;

    QPtrListIterator<QTextCodec> it(codecs);
    while ((codec = it.current())) {
	++it;

	if (codec->name() == name)
	    break;
    }

    if (! codec) {
	if (name == "JIS7")
	    codec = new QJisCodec;
	else if (name == "SJIS")
	    codec = new QSjisCodec;
	else if (name == "eucJP")
	    codec = new QEucJpCodec;
	else if (name == "jisx0208.1983-0")
	    codec = new QFontJis0208Codec;

	if (codec)
	    codecs.append(codec);
    }

    return codec;
}


Q_EXPORT_PLUGIN( JPTextCodecs );
