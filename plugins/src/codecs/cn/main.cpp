#include <qtextcodecinterface.h>
#include <qtextcodec.h>
#include <qptrlist.h>
#include <qapplication.h>

#include <qgbkcodec.h>
#include <private/qfontcodecs_p.h>


class CNTextCodecs : public QTextCodecFactoryInterface
{
public:
    CNTextCodecs();
    virtual ~CNTextCodecs();

    // unknown interface
    QRESULT queryInterface(const QUuid &, QUnknownInterface **);
    Q_REFCOUNT;

    // feature list interface
    QStringList featureList() const;

    // text codec interface
    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );


private:
    QPtrList<QTextCodec> codecs;
};


CNTextCodecs::CNTextCodecs()
{
}


CNTextCodecs::~CNTextCodecs()
{
}


QRESULT CNTextCodecs::queryInterface(const QUuid &uuid, QUnknownInterface **iface)
{
    *iface = 0;

    if (uuid == IID_QUnknown )
	*iface = (QUnknownInterface *) this;
    else if (uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface *) this;
    else if (uuid == IID_QTextCodecFactory )
	*iface = (QTextCodecFactoryInterface*) this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
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

    QPtrListIterator<QTextCodec> it(codecs);
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

    QPtrListIterator<QTextCodec> it(codecs);
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


Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( CNTextCodecs );
}
