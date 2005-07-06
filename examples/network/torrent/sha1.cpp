/****************************************************************************
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <QtCore/QByteArray>

// array of constants K[0] do K[79]
static const unsigned long int K[] = {
    0x5A827999UL, 0x5A827999UL, 0x5A827999UL, 0x5A827999UL, 0x5A827999UL,
    0x5A827999UL, 0x5A827999UL, 0x5A827999UL, 0x5A827999UL, 0x5A827999UL,
    0x5A827999UL, 0x5A827999UL, 0x5A827999UL, 0x5A827999UL, 0x5A827999UL,
    0x5A827999UL, 0x5A827999UL, 0x5A827999UL, 0x5A827999UL, 0x5A827999UL,
    0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL, 
    0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL,
    0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL, 
    0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL, 0x6ED9EBA1UL,
    0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL, 
    0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL,
    0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL, 
    0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL, 0x8F1BBCDCUL,
    0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL, 
    0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL,
    0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL, 
    0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL, 0xCA62C1D6UL};

inline unsigned long int f(int t, unsigned long int B, unsigned long int C, unsigned long int D)
{
    unsigned long int retval = 0;

    if ( t <= 19 )
        retval = (B & C) | ((~B) & D);
    else if ( t >= 20 && t <= 39 )
        retval = B ^ C ^ D;
    else if ( t >= 40 && t <= 59 )
        retval = (B & C) | (B & D) | (C & D);
    else if ( t >= 60 && t <=79 )
        retval = B ^ C ^ D;
    return retval;
}

inline unsigned long int cshift(unsigned long int source, int shift)
{
    return ((source << shift) | (source >> (32 - shift)) & Q_UINT64_C(0xffffffff));
}

#define big2littlelong(src,dst) \
    *((unsigned long int *)dst) \
	= ((src & 0xff) << 24) \
	| ((src & 0xff00) << 8) \
	| ((src & 0xff0000) >> 8) \
	| ((src & 0xff000000) >> 24)

#define big2littlelonglong(src,dst) \
    *((unsigned long long int *)dst) \
	= ((src & 0xff) << 56)\
	| ((src & 0xff00) << 40)\
	| ((src & 0xff0000) << 24)\
	| ((src & 0xff000000) << 8)\
	| ((src & Q_UINT64_C(0xff00000000)) >> 8)\
	| ((src & Q_UINT64_C(0xff0000000000)) >> 24)\
	| ((src & Q_UINT64_C(0xff000000000000)) >> 40)\
	| ((src & Q_UINT64_C(0xff00000000000000)) >> 56)

QByteArray sha1Checksum(const QByteArray &message)
{
    unsigned long long int originalLen = message.size();

    unsigned long long int woPadding = originalLen + 8;
    unsigned long long int blocks = woPadding / 64;
    unsigned long long int reminder = woPadding % 64;

    unsigned int padSize = 64 - reminder;

    QByteArray paddedMessage;
    paddedMessage.resize( blocks * 64 + padSize + reminder);
    char *it = paddedMessage.data();

    // copying original byte array
    if ( originalLen > 0 ){
        memmove( it, message.constData(), originalLen );
        it += originalLen;
    }

    // adding up padding
    if ( padSize > 0 ){
        (*it) = 0x80;
        it++;
        for ( unsigned int count = 1; count < padSize; count++ ){
            (*it) = 0x00;
            it++;
        }
    }

    // putting the 8 byte integer to the end of the array that represent
    // original size of the message - it is expected to be in big endian
    // notation so this is platform dependant
    unsigned long long int lenInBits = originalLen * 8;
    big2littlelonglong(lenInBits, it);

    // I've turned whole message into little endian so I can manipulate blocks
    // as long ints
    int wordCount = paddedMessage.size() / 4;
    it = paddedMessage.data();

    for ( int word = 0; word < wordCount; word++ ){
        unsigned long int source = (* (unsigned long int *)it );
        big2littlelong(source, it);
        it += 4;
    }

    // initializing seed values
    unsigned long int H[5] = { 0x67452301UL, 0xEFCDAB89UL, 0x98BADCFEUL, 0x10325476UL, 0xC3D2E1F0UL };

    // calculating message digest
    int blockCount = paddedMessage.size() / 64;
    it = paddedMessage.data();

    unsigned long int mask32 = 0xffffffffUL;
    unsigned long int *w[16];

    for (int block = 0; block < blockCount; ++block){

        (*w) = (unsigned long int*)it;
        unsigned long int W[80];

        for (int word = 0; word < 16; ++word)
            W[word] = (*w)[word];

        for (int word = 16; word < 80; ++word)
            W[word] = cshift ( (W[word-3] ^ W[word-8] ^ W[word-14] ^ W[word-16]), 1);

        unsigned long int A = H[0];
        unsigned long int B = H[1];
        unsigned long int C = H[2];
        unsigned long int D = H[3];
        unsigned long int E = H[4];

        for (int word = 0; word < 80; ++word) {
            unsigned long int TEMP = ( cshift(A,5) + f(word, B, C, D) + E + W[word] + K[word] ) & mask32;

            E = D;
            D = C;
            C = cshift(B,30);
            B = A;
            A = TEMP;
        }

        H[0] = ( H[0] + A ) & mask32;
        H[1] = ( H[1] + B ) & mask32;
        H[2] = ( H[2] + C ) & mask32;
        H[3] = ( H[3] + D ) & mask32;
        H[4] = ( H[4] + E ) & mask32;

        it += 64;
    }

    QByteArray sha1Sum(20, ' ');
    unsigned char *digest = (unsigned char *)H;
    for (int i = 0; i < 5; ++i) {
	sha1Sum[i * 4 + 0] = digest[i * 4 + 3];
	sha1Sum[i * 4 + 1] = digest[i * 4 + 2];
	sha1Sum[i * 4 + 2] = digest[i * 4 + 1];
	sha1Sum[i * 4 + 3] = digest[i * 4 + 0];
    }
    
    return sha1Sum;
}
