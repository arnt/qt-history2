/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Palmtop Environment.
**
** Licensees holding valid Qt Palmtop Developer license may use this 
** file in accordance with the Qt Palmtop Developer License Agreement 
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
** PURPOSE.
**
** email sales@trolltech.com for information about Qt Palmtop License 
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "qrsync.h"
#include <stdio.h>
extern "C" {
#include "rsync.h"
#include "checksum.h"
}
#include <stdlib.h>

#include <qdir.h>
#include <qfile.h>


QMd4Sum::QMd4Sum( QString s )
{
    s = s.simplifyWhiteSpace();
    rs_calc_strong_sum( (void *)s.unicode(), s.length()*sizeof( QChar ), (rs_strong_sum_t *)&md4sum.uchars );   
}
 
const QMd4Sum &QMd4Sum::operator = ( const QString &s )
{
    rs_calc_strong_sum( (void *)s.unicode(), s.length()*sizeof( QChar ), (rs_strong_sum_t *)&md4sum.uchars );   
    return *this;
}

static const char *rdiffNewFile = "/tmp/rdiff/result";
static size_t block_len = RS_DEFAULT_BLOCK_LEN;
static size_t strong_len = RS_DEFAULT_STRONG_LEN;


void QRsync::generateSignature( QString baseFile, QString sigFile )
{

    if ( QFile::exists( baseFile ) ) {
	FILE *basis_file, *sig_file;
	rs_result       result;
	
	basis_file = fopen( baseFile.latin1(), "rb" );
	sig_file = fopen( sigFile.latin1(), "wb" );

	result = rs_sig_file(basis_file, sig_file, block_len, strong_len, 0);

	fclose( basis_file );
	fclose( sig_file );
	if (result != RS_DONE)
	    printf("error in rdiffGenSig: %d", result );
    }
}


void QRsync::generateDiff( QString baseFile, QString sigFile, QString deltaFile )
{
    if ( QFile::exists( baseFile ) && QFile::exists( sigFile ) ) {
	FILE            *sig_file, *new_file, *delta_file;
	rs_result       result;
	rs_signature_t  *sumset;

	sig_file = fopen(sigFile.latin1(), "rb");
	new_file = fopen(baseFile.latin1(), "rb");
	delta_file = fopen(deltaFile.latin1(), "wb");

	result = rs_loadsig_file(sig_file, &sumset, 0);
	if (result != RS_DONE) {
	    qDebug( "rdiffGenDiff: loading of sig file failed, error=%d", result );
	    return;
	}

	result = rs_build_hash_table(sumset);
	if ( result != RS_DONE) {
	    qDebug( "rdiffGenDiff: building of hash table failed, error=%d", result );
	    return;
	}

	result = rs_delta_file(sumset, new_file, delta_file, 0);
	if ( result != RS_DONE) {
	    qDebug( "rdiffGenDiff: writing of diff file failed, error=%d", result );
	    return;
	}

	fclose( new_file );
	fclose( delta_file );
	fclose( sig_file );

    }
}

void QRsync::applyDiff( QString baseFile, QString deltaFile )
{
    if ( QFile::exists( baseFile ) && QFile::exists( deltaFile ) ) {
	FILE               *basis_file, *delta_file, *new_file;
	rs_result           result;

	basis_file = fopen(baseFile.latin1(), "rb");
	delta_file = fopen(deltaFile.latin1(), "rb");
#ifdef Q_WS_WIN
	new_file = fopen( (baseFile + ".new").latin1(), "wb" );
#else
	new_file =   fopen(rdiffNewFile, "wb");
#endif	

	result = rs_patch_file(basis_file, delta_file, new_file, 0);
	if (result != RS_DONE) {
	    qDebug( "rdiffApplyDiff failed with result %d", result );
	    return;
	}

	fclose( basis_file );
	fclose( delta_file );
	fclose( new_file );

#ifdef Q_WS_WIN	
	QDir dir;
	QString backup = baseFile + "~";
	dir.rename( baseFile, backup );
	dir.rename( (baseFile + ".new"), baseFile );
	dir.remove( backup );
#else
	QString cmd = "mv ";
	cmd += rdiffNewFile;
	cmd += " " + baseFile;
	system( cmd.latin1() );
#endif
    }

}
