#include <qtextcodecinterface.h>
#include <qtextcodec.h>
#include <qlist.h>

#include <qgbkcodec.h>
#include "../../qfontcodecs_p.h"


class CNTextCodecs : public QTextCodecInterface
{
public:
    CNTextCodecs();
    virtual ~CNTextCodecs();

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


CNTextCodecs::CNTextCodecs()
    : ref(0)
{
}


CNTextCodecs::~CNTextCodecs()
{
}


QUnknownInterface *CNTextCodecs::queryInterface(const QUuid &uuid)
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


unsigned long CNTextCodecs::addRef()
{
    return ref++;
}


unsigned long CNTextCodecs::release()
{
    if (! --ref) {
	delete this;
	return 0;
    }

    return ref;
}


QStringList CNTextCodecs::featureList() const
{
    QStringList list;
    list << "GBK" << "gb2312.1980-0";
    list << "MIB-2027" << "MIB-57";
    return list;
}


QTextCodec *CNTextCodecs::createForMib( int mib )
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
	case 57:
	    codec = new QFontGB2312Codec;
	    break;

	case 2027:
	    codec = new QGbkCodec;
	    break;

	default:
	    ;
	}

	if (codec)
	    codecs.append(codec);
    }

    return codec;
}


QTextCodec *CNTextCodecs::createForName( const QString &name )
{
    QTextCodec *codec = 0;

    QListIterator<QTextCodec> it(codecs);
    while ((codec = it.current())) {
	++it;

	if (codec->name() == name)
	    break;
    }

    if (! codec) {
	if (name == "GBK")
	    codec = new QGbkCodec;
	else if (name == "gb2312.1980-0")
	    codec = new QFontGB2312Codec;

	if (codec)
	    codecs.append(codec);
    }

    return codec;
}


Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface *) new CNTextCodecs;
    iface->addRef();
    return iface;
}
