#include <qtextcodecinterface.h>
#include <qtextcodec.h>
#include <qlist.h>

#include <qeucjpcodec.h>
#include <qjiscodec.h>
#include <qsjiscodec.h>
#include "../../qfontcodecs_p.h"


class JPTextCodecs : public QTextCodecInterface
{
public:
    JPTextCodecs();
    virtual ~JPTextCodecs();

    // unknown interface
    QUnknownInterface *queryInterface(const QUuid &);
    unsigned long addRef();
    unsigned long release();

    // feature list interface
    QStringList featureList() const;

    // text codec interface
    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );


private:
    QList<QTextCodec> codecs;

    unsigned long ref;
};


JPTextCodecs::JPTextCodecs()
    : ref(0)
{
}


JPTextCodecs::~JPTextCodecs()
{
}


QUnknownInterface *JPTextCodecs::queryInterface(const QUuid &uuid)
{
    QUnknownInterface *iface = 0;
    if (uuid == IID_QUnknownInterface)
	iface = (QUnknownInterface *) this;
    else if (uuid == IID_QFeatureListInterface)
	iface = (QFeatureListInterface *) this;
    else if (uuid == IID_QTextCodecInterface)
	iface = (QTextCodecInterface*) this;

    if (iface)
	iface->addRef();
    return iface;
}


unsigned long JPTextCodecs::addRef()
{
    return ref++;
}


unsigned long JPTextCodecs::release()
{
    if (! --ref) {
	delete this;
	return 0;
    }

    return ref;
}


QStringList JPTextCodecs::featureList() const
{
    QStringList list;
    list << "eucJP" << "JIS7" << "SJIS" << "jisx0208.1983-0" << "jisx0212.1990-0";
    list << "MIB-16" << "MIB-17" << "MIB-18" << "MIB-63" << "MIB-98";
    return list;
}


QTextCodec *JPTextCodecs::createForMib( int mib )
{
    QTextCodec *codec = 0;

    QListIterator<QTextCodec> it(codecs);
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

	case 98:
	    codec = new QFontJis0212Codec;
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

    QListIterator<QTextCodec> it(codecs);
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
	else if (name == "jisx0212.1990-0")
	    codec = new QFontJis0212Codec;

	if (codec)
	    codecs.append(codec);
    }

    return codec;
}


Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface *) new JPTextCodecs;
    iface->addRef();
    return iface;
}
