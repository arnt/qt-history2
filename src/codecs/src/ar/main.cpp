#include <qtextcodecinterface.h>
#include <qtextcodec.h>
#include <qptrlist.h>

#include "../../qfontcodecs_p.h"


class ARTextCodecs : public QTextCodecInterface
{
public:
    ARTextCodecs();
    virtual ~ARTextCodecs();

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


ARTextCodecs::ARTextCodecs()
    : ref(0)
{
}


ARTextCodecs::~ARTextCodecs()
{
}


void ARTextCodecs::queryInterface(const QUuid &uuid, QUnknownInterface **iface)
{
    if (uuid == IID_QUnknownInterface)
	*iface = (QUnknownInterface *) this;
    else if (uuid == IID_QFeatureListInterface)
	*iface = (QFeatureListInterface *) this;
    else if (uuid == IID_QTextCodecInterface)
	*iface = (QTextCodecInterface*) this;

    if (*iface)
	(*iface)->addRef();
}


unsigned long ARTextCodecs::addRef()
{
    return ref++;
}


unsigned long ARTextCodecs::release()
{
    if (! --ref) {
	delete this;
	return 0;
    }

    return ref;
}


QStringList ARTextCodecs::featureList() const
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


Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface *) new ARTextCodecs;
    iface->addRef();
    return iface;
}
