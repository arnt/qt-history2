#include <qdict.h>
#include <zlib.h>    


struct Embed {
    unsigned long	compressedSize;
    unsigned long       uncompressedSize;
    const unsigned char *data;
    const char          *name;
};


extern Embed embed_vec[];


unsigned long long embeddedDataSize( const QString& name )
{
    for ( int i = 0; embed_vec[i].data; i++ ) 
	if ( name == embed_vec[i].name ) 
	    return embed_vec[i].uncompressedSize;
    return 0;
}


const QByteArray& embeddedData( const QString& name )
{
    static QDict<QByteArray> dict;
    QByteArray *ba = dict.find(name);
    
    if ( !ba ) {
        for (int i=0; embed_vec[i].data; i++) {
	    if ( name == embed_vec[i].name ) {
		unsigned char *uncompressedBuffer = new unsigned char[ embed_vec[i].uncompressedSize + 100 ];
		uncompress( uncompressedBuffer, &embed_vec[i].uncompressedSize, embed_vec[i].data, embed_vec[i].compressedSize );
		ba = new QByteArray;
		ba->setRawData( (char*)uncompressedBuffer, embed_vec[i].uncompressedSize );
		dict.insert(name, ba);
		break;
	    }
        }
        if ( !ba ) {
            static QByteArray dummy;
            return dummy;
        }
    }
    return *ba;
}

