#include <qtextcodecinterface.h>
#include <qtextcodec.h>
#include <qptrlist.h>
#include <qapplication.h>

#include <qbig5codec.h>
#include <private/qfontcodecs_p.h>


class TWTextCodecs : public QTextCodecFactoryInterface
{
public:
    TWTextCodecs();
    virtual ~TWTextCodecs();

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


TWTextCodecs::TWTextCodecs()
{
}


TWTextCodecs::~TWTextCodecs()
{
}

QRESULT TWTextCodecs::queryInterface(const QUuid &uuid, QUnknownInterface **iface)
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


Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( TWTextCodecs );
}
