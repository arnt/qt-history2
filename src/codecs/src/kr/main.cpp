#include <qtextcodecinterface.h>
#include <qtextcodec.h>
#include <qlist.h>

#include <qeuckrcodec.h>
#include "../../qfontcodecs_p.h"


class KRTextCodecs : public QTextCodecInterface
{
public:
    KRTextCodecs();
    virtual ~KRTextCodecs();

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


KRTextCodecs::KRTextCodecs()
    : ref(0)
{
}


KRTextCodecs::~KRTextCodecs()
{
}


QUnknownInterface *KRTextCodecs::queryInterface(const QUuid &uuid)
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


unsigned long KRTextCodecs::addRef()
{
    return ref++;
}


unsigned long KRTextCodecs::release()
{
    if (! --ref) {
	delete this;
	return 0;
    }

    return ref;
}


QStringList KRTextCodecs::featureList() const
{
    QStringList list;
    list << "eucKR" << "ksc5601.1987-0" << "ksc5601.1992-*";
    list << "MIB-38" << "MIB-36";
    return list;
}


QTextCodec *KRTextCodecs::createForMib( int mib )
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

    QListIterator<QTextCodec> it(codecs);
    while ((codec = it.current())) {
	++it;

	if (codec->name() == name)
	    break;
    }

    if (! codec) {
	if (name == "eucKR")
	    codec = new QEucKrCodec;
	else if (name == "ksc5601.1987-0" ||
		 name == "ksc5601.1992-*")
	    codec = new QFontKsc5601Codec;

	if (codec)
	    codecs.append(codec);
    }

    return codec;
}


Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface *) new KRTextCodecs;
    iface->addRef();
    return iface;
}


