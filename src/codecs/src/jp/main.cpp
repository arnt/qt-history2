#include <qtextcodecinterface.h>
#include <qtextcodec.h>
#include <qptrlist.h>

#include <qeucjpcodec.h>
#include <qjiscodec.h>
#include <qsjiscodec.h>
#include "../../qfontcodecs_p.h"


class JPTextCodecs : public QTextCodecFactoryInterface
{
public:
    JPTextCodecs();
    virtual ~JPTextCodecs();

    // unknown interface
    void queryInterface(const QUuid &, QUnknownInterface **);
    unsigned long addRef();
    unsigned long release();

    // feature list interface
    QStringList featureList() const;

    // text codec interface
    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );


private:
    QPtrList<QTextCodec> codecs;

    unsigned long ref;
};


JPTextCodecs::JPTextCodecs()
    : ref(0)
{
}


JPTextCodecs::~JPTextCodecs()
{
}


void JPTextCodecs::queryInterface(const QUuid &uuid, QUnknownInterface **iface)
{
    if (uuid == IID_QUnknown )
	*iface = (QUnknownInterface *) this;
    else if (uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface *) this;
    else if (uuid == IID_QTextCodecFactory )
	*iface = (QTextCodecFactoryInterface*) this;

    if (*iface)
	(*iface)->addRef();
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
