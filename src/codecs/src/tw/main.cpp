#include <qtextcodecinterface.h>
#include <qtextcodec.h>
#include <qptrlist.h>

#include <qbig5codec.h>
#include "../../qfontcodecs_p.h"


class TWTextCodecs : public QTextCodecFactoryInterface
{
public:
    TWTextCodecs();
    virtual ~TWTextCodecs();

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


TWTextCodecs::TWTextCodecs()
    : ref(0)
{
}


TWTextCodecs::~TWTextCodecs()
{
}


void TWTextCodecs::queryInterface(const QUuid &uuid, QUnknownInterface **iface)
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


unsigned long TWTextCodecs::addRef()
{
    return ref++;
}


unsigned long TWTextCodecs::release()
{
    if (! --ref) {
	delete this;
	return 0;
    }

    return ref;
}


QStringList TWTextCodecs::featureList() const
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


Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface *) new TWTextCodecs;
    iface->addRef();
    return iface;
}
