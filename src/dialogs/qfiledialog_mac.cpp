#include "qfiledialog.h"

#ifndef QT_NO_FILEDIALOG

#include "qapplication.h"
#include <private/qapplication_p.h>
#include "qt_mac.h"
#include "qregexp.h"
#include "qbuffer.h"
#include "qstringlist.h"
#include "qtextcodec.h"

static UInt8 *str_buffer = NULL;
static void cleanup_str_buffer()
{
    if(str_buffer) {
	free(str_buffer);
	str_buffer = NULL;
    }
}

extern const char qt_file_dialog_filter_reg_exp[]; // defined in qfiledialog.cpp

// Returns the wildcard part of a filter.
static QString extractFilter( const QString& rawFilter )
{
    QString result = rawFilter;
    QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
    int index = r.search( result );
    if ( index >= 0 )
	result = r.cap( 1 );
    return result.replace( QRegExp(QString::fromLatin1(" ")), QChar(';') );
}

// Makes a list of filters from ;;-separated text.
static QPtrList<QRegExp> makeFiltersList( const QString &filter )
{
    QString f( filter );

    if ( f.isEmpty( ) )
	f = QFileDialog::tr( "All Files (*.*)" );

    if ( f.isEmpty() )
	return QPtrList<QRegExp>();

    int i = f.find( ";;", 0 );
    QString sep( ";;" );
    if ( i == -1 ) {
	if ( f.find( "\n", 0 ) != -1 ) {
	    sep = "\n";
	    i = f.find( sep, 0 );
	}
    }

    QPtrList<QRegExp> ret;
    QStringList filts = QStringList::split( sep, f);
    for (QStringList::Iterator it = filts.begin(); it != filts.end(); ++it )
	ret.append(new QRegExp(extractFilter((*it)), TRUE, TRUE));
    return ret;
}


QMAC_PASCAL static Boolean qt_mac_nav_filter(AEDesc *theItem, void *info,
				 void *myd, NavFilterModes)
{
    QPtrList<QRegExp> *filt = (QPtrList<QRegExp> *)myd;
    if(!filt)
	return true;

    NavFileOrFolderInfo *theInfo = (NavFileOrFolderInfo *)info;
    if(theItem->descriptorType == typeFSS ) {
	if( !theInfo->isFolder ) {
	    AliasHandle alias;
	    Str63 str;
	    char tmp[sizeof(Str63)+2];
	    FSSpec      FSSpec;
	    AliasInfoType x = 0;

	    AEGetDescData( theItem, &FSSpec, sizeof(FSSpec));

	    if(NewAlias( NULL, &FSSpec, &alias ) != noErr)
		return true;
	    GetAliasInfo(alias, (AliasInfoType)x++, str);
	    if(str[0]) {
		strncpy((char *)tmp, (const char *)str+1, str[0]);
		tmp[str[0]] = '\0';

		for (QPtrListIterator<QRegExp> it(*filt); it.current(); ++it ) {
		    if(it.current()->exactMatch( tmp ))
			return true;
		}
	    }
	    return false;
	}
    }
    return true;
}
static NavObjectFilterUPP mac_navUPP = NULL;
static void cleanup_navUPP()
{
    DisposeNavObjectFilterUPP(mac_navUPP);
    mac_navUPP = NULL;
}
static const NavObjectFilterUPP make_navUPP()
{
    if(mac_navUPP)
	return mac_navUPP;
    qAddPostRoutine( cleanup_navUPP );
    return mac_navUPP = NewNavObjectFilterUPP(qt_mac_nav_filter);
}

const unsigned char * p_str(const char *, int len=-1);
QMAC_PASCAL OSErr FSpLocationFromFullPath( short fullPathLength,
				      const void *fullPath,
				      FSSpec *spec);

QString QFileDialog::macGetOpenFileName( const QString &/*initialSelection*/,
					 const QString &filter,
					 QString* initialDirectory,
					 QWidget *parent, const char* name,
					 const QString& caption )
{
    return macGetOpenFileNames(filter, initialDirectory,
			       parent, name, caption).first();
}

QStringList QFileDialog::macGetOpenFileNames( const QString &filter,
					      QString* initialDirectory,
					      QWidget *parent,
					      const char* /*name*/,
					      const QString& caption )
{
    OSErr err;
    QString tmpstr;
    QStringList retstrl;
    NavDialogOptions options;
    NavGetDefaultDialogOptions( &options );
    options.version = kNavDialogOptionsVersion;
    options.location.h = options.location.v = -1;
    if(parent)
	strcpy((char *)options.clientName,
	       (const char *) p_str(parent->caption()));
    if(caption.length())
	strcpy((char *)options.windowTitle,
	       (const char *)p_str(caption));

    bool use_initial = FALSE;
    AEDesc initial;
    if(initialDirectory) {
	QString macFilename = initialDirectory->mid( 1 );
	while ( macFilename.find( "/" ) != -1 )
	    macFilename.replace( macFilename.find( "/" ), 1, ":" );
	//FIXME: prepend the volume name to the macFilename

	FSSpec fileSpec;
	err = FSpLocationFromFullPath( macFilename.length(),
				       macFilename.latin1(), &fileSpec );
	if(err == noErr) {
	    err = AECreateDesc(typeFSS, &fileSpec, sizeof(fileSpec), &initial );
	    if(err == noErr)
		use_initial = TRUE;
	}
    }

    NavReplyRecord ret;
    QPtrList<QRegExp> filts = makeFiltersList(filter);
    NavGetFile(use_initial ? &initial : NULL, &ret, &options, NULL, NULL,
	       make_navUPP(), NULL, (void *) (filts.isEmpty() ? NULL : &filts));
    filts.setAutoDelete(TRUE);
    filts.clear();

    long count;
    err = AECountItems(&(ret.selection), &count);

    if(!ret.validRecord || err != noErr || !count)
	goto get_name_out;

    AEKeyword	keyword;
    DescType    type;
    Size        size;
    FSSpec      FSSpec;

    for(long index = 1; index <= count; index++) {
	err = AEGetNthPtr(&(ret.selection), index, typeFSS, &keyword,
			  &type,&FSSpec, sizeof(FSSpec), &size);
	if(err != noErr)
	    goto get_name_out;

	//we must *try* to create a file, and remove it if successfull
	//to actually get a path, bogus? I think so.
	err = FSpCreate(&FSSpec, 'CUTE', 'TEXT', smSystemScript);
	FSRef ref;
	FSpMakeFSRef(&FSSpec, &ref);
	if(!str_buffer) {
	    qAddPostRoutine( cleanup_str_buffer );
	    str_buffer = (UInt8 *)malloc(1024);
	}
	FSRefMakePath(&ref, str_buffer, 1024);
	if(err == noErr) 
	    FSpDelete(&FSSpec);
	tmpstr = QString::fromUtf8((const char *)str_buffer);
	retstrl.append(tmpstr);
    }

 get_name_out:
    if(use_initial)
	AEDisposeDesc(&initial);
    NavDisposeReply(&ret);
    return retstrl;
}

QString QFileDialog::macGetSaveFileName( const QString &,
					 const QString &filter,
					 QString* initialDirectory,
					 QWidget *parent, const char* /*name*/,
					 const QString& caption )
{
    OSErr err;
    QString retstr;
    NavDialogOptions options;
    NavGetDefaultDialogOptions( &options );
    options.version = kNavDialogOptionsVersion;
    options.location.h = options.location.v = -1;
    if(parent)
	strcpy((char *)options.clientName,
	       (const char *) p_str(parent->caption()));
    if(caption.length())
	strcpy((char *)options.windowTitle,
	       (const char *)p_str(caption));

    bool use_initial = FALSE;
    AEDesc initial;
    if(initialDirectory) {
	QString macFilename = initialDirectory->mid( 1 );
	while ( macFilename.find( "/" ) != -1 )
	    macFilename.replace( macFilename.find( "/" ), 1, ":" );
	//FIXME: prepend the volume name to the macFilename

	FSSpec fileSpec;
	err = FSpLocationFromFullPath( macFilename.length(),
				       macFilename.latin1(), &fileSpec );
	if(err == noErr) {
	    err = AECreateDesc(typeFSS, &fileSpec, sizeof(fileSpec), &initial );
	    if(err == noErr)
		use_initial = TRUE;
	}
    }

    NavReplyRecord ret;
    QPtrList<QRegExp> filts = makeFiltersList(filter);
    NavPutFile(use_initial ? &initial : NULL, &ret, &options, NULL,
	       'cute', kNavGenericSignature, (void *) filts.isEmpty() ? NULL : &filts);
    filts.setAutoDelete(TRUE);
    filts.clear();

    long count;
    err = AECountItems(&(ret.selection), &count);

    if(!ret.validRecord || err != noErr || !count)
	goto put_name_out;

    AEKeyword	keyword;
    DescType    type;
    Size        size;
    FSSpec      FSSpec;

    err = AEGetNthPtr(&(ret.selection), 1, typeFSS, &keyword,
		      &type,&FSSpec, sizeof(FSSpec), &size);
    if(err != noErr)
	goto put_name_out;

    //we must *try* to create a file, and remove it if successfull
    //to actually get a path, bogus? I think so.
    err = FSpCreate(&FSSpec, 'CUTE', 'TEXT', smSystemScript);
    FSRef ref;
    FSpMakeFSRef(&FSSpec, &ref);
    if(!str_buffer) {
	qAddPostRoutine( cleanup_str_buffer );
	str_buffer = (UInt8 *)malloc(1024);
    }
    FSRefMakePath(&ref, str_buffer, 1024);
    if(err == noErr) 
	FSpDelete(&FSSpec);
    retstr = QString::fromUtf8((const char *)str_buffer);

 put_name_out:
    if(use_initial)
	AEDisposeDesc(&initial);
    NavDisposeReply(&ret);
    return retstr;
}

#endif
