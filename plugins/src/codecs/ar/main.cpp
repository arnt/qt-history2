#include <qtextcodecinterface.h>
#include <qtextcodec.h>
#include <qptrlist.h>
#include <qapplication.h>

#include <private/qfontcodecs_p.h>


class ARTextCodecs : public QTextCodecFactoryInterface
{
public:
    ARTextCodecs();
    virtual ~ARTextCodecs();

    // unknown interface
    QRESULT queryInterface(const QUuid &, QUnknownInterface **);
    Q_REFCOUNT

    // feature list interface
    QStringList featureList() const;

    // text codec interface
    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );

private:
    QPtrList<QTextCodec> codecs;
};


ARTextCodecs::ARTextCodecs()
{
}


ARTextCodecs::~ARTextCodecs()
{
}


QRESULT ARTextCodecs::queryInterface(const QUuid &uuid, QUnknownInterface **iface)
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


Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( ARTextCodecs );
}
