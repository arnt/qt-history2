#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qptrlist.h>
#include <qapplication.h>

#include <qgbkcodec.h>
#include <private/qfontcodecs_p.h>


class CNTextCodecs : public QTextCodecPlugin
{
public:
    CNTextCodecs() {}

    QStringList names() const { return QStringList() << "GBK" << "gb2312.1980-0"; }
    QValueList<int> mibEnums() const { return QValueList<int>() << 57 << 2027; }

    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );
};

QTextCodec *CNTextCodecs::createForMib( int mib )
{
    switch (mib) {
    case 57:
	return new QFontGB2312Codec;
    case 2027:
	return new QGbkCodec;
    default:
	;
    }

    return 0;
}


QTextCodec *CNTextCodecs::createForName( const QString &name )
{
    if (name == "GBK")
	return new QGbkCodec;
    if (name == "gb2312.1980-0")
	return new QFontGB2312Codec;

    return 0;
}


Q_EXPORT_PLUGIN( CNTextCodecs );
