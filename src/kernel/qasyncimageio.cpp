/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qasyncimageio.cpp#17 $
**
** Implementation of asynchronous image/movie loading classes
**
** Created : 970617
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include <qpainter.h>
#include <qlist.h>
#include "qasyncimageio.h"

#include <stdlib.h>

/*!
  \class QImageConsumer qasyncimageio.h
  \brief An abstraction used by QImageDecoder.

  \internal

  A QImageConsumer consumes information about changes to the
  QImage maintained by a QImageDecoder.  It represents the
  decoder's view of the obhect which uses the image it produces.
  Each method of QImageConsumer returns a bool which should be
  TRUE if the decoder should continue decoding.

  \sa QImageDecoder
*/

/*!
  \fn bool QImageConsumer::changed(const QRect&)

  Called when the given area of the image has changed.
*/

/*!
  \fn bool QImageConsumer::end()

  Called when all data of all frames has been decoded and revealed
  as changed().
*/

/*!
  \fn bool QImageConsumer::frameDone()

  Called when a frame of an animated image has ended and been revealed
  as changed().  If the consumer returns TRUE, the decoder will not make
  any further changes to the image until the next call to
  QImageFormatDecoder::decode().
*/

/*!
  \fn bool QImageConsumer::setLooping(int n)

  Called to indicate that the sequence of frames in the image
  should be repeated \a n times, including the sequence during
  decoding.

  <ul>
    <li> 0 = Forever
    <li> 1 = Only display frames the first time through
    <li> 2 = Repeat once after first pass through images
    <li> etc.
  </ul>

  To make the QImageDecoder
  do this just delete it and pass the information to it again
  for decoding (setLooping() will be called again of course, but
  that can be ignored), or keep copies of the
  changed areas at the ends of frames.
*/

/*!
  \fn bool QImageConsumer::setFramePeriod(int milliseconds)

  Notes that the frame about to be decoded should not be displayed until
  the given number of \a milliseconds after the time that this function
  is called.  Of course, the image may not have been decoded by then, in
  which case the frame should not be displayed until it is complete.
  A value of -1 (the assumed default) indicates that the image should
  be diplayed even while it is only partially loaded.
*/

/*!
  \fn bool QImageConsumer::setSize(int, int)

  This function is called as soon as the size of the image has
  been determined.
*/


/*!
  \class QImageDecoder qasyncimageio.h
  \brief Incremental image decoder for all supported image formats.

  \internal

  New formats are installed by creating objects of class
  QImageFormatDecoderFactory.
*/

static const int max_header = 32;

struct QImageDecoderPrivate {
    QImageDecoderPrivate()
    {
	count = 0;
    }

    static QList<QImageFormatDecoderFactory> factories;

    uchar header[max_header];
    int count;
};
QList<QImageFormatDecoderFactory> QImageDecoderPrivate::factories;

/*!
  Constructs a QImageDecoder which will send change information to
  a given QImageConsumer.
*/
QImageDecoder::QImageDecoder(QImageConsumer* c)
{
    d = new QImageDecoderPrivate;
    CHECK_PTR(d);
    consumer = c;
    actual_decoder = 0;
}

/*!
  Destroys a QImageDecoder.  The image it built is destroyed.  The decoder
  built by the factory for the file format is destroyed. The consumer
  for which it decoded the image is \e not destroyed.
*/
QImageDecoder::~QImageDecoder()
{
    delete d;
    delete actual_decoder;
}

/*!
  \fn const QImage& QImageDecoder::image()

  Returns the image currently being decoded.
*/

/*!
  Call this function to decode some data into image changes.  The data
  will be decoded, sending change information to the QImageConsumer of
  this QImageDecoder, until one of the change functions of the consumer
  returns FALSE.

  Returns the number of bytes consumed, 0 if consumption is complete,
  and -1 if decoding fails dur to invalid data.
*/
int QImageDecoder::decode(const uchar* buffer, int length)
{
    if (actual_decoder) {
	return actual_decoder->decode(img, consumer, buffer, length);
    } else {
	int consumed=0;
	while (consumed < length && d->count < max_header) {
	    d->header[d->count++] = buffer[consumed++];
	}

	for (QImageFormatDecoderFactory* f = QImageDecoderPrivate::factories.first();
	    f && !actual_decoder;
	    f = QImageDecoderPrivate::factories.next())
	{
	    actual_decoder = f->decoderFor(d->header, d->count);
	}

	if (actual_decoder) {
	    uchar* b = d->header;
	    int more = 1;
	    while (d->count > 0)  {
		more = actual_decoder->decode(img, consumer, b, d->count);
		if ( more <= 0 ) break;
		d->count -= more;
		b += more;
	    }
	    if (more <= 0) {
		// Decoder must have failed.  Input not valid.  We assume
		// consumer has been notified.
		delete actual_decoder;
		actual_decoder = 0;
		return more;
	    }
	}

	return consumed;
    }
}

/*!
  Call this function to find the name of the format of the given header.
  The returned string is statically allocated.

  Returns 0 if the format is not recognized.
*/
const char* QImageDecoder::formatName(const uchar* buffer, int length)
{
    const char* name = 0;
    for (QImageFormatDecoderFactory* f = QImageDecoderPrivate::factories.first();
	f && !name;
	f = QImageDecoderPrivate::factories.next())
    {
	QImageFormatDecoder *decoder = f->decoderFor(buffer, length);
	if (decoder) {
	    name = f->formatName();
	    delete decoder;
	}
    }
    return name;
}

/*!
  Returns a sorted list of formats for which asynchronous loading is supported.
*/
QStrList QImageDecoder::inputFormats()
{
    QStrList result;

    for (QImageFormatDecoderFactory* f = QImageDecoderPrivate::factories.first();
	f; f = QImageDecoderPrivate::factories.next())
    {
	if ( !result.contains(  f->formatName() ) ) {
	    result.inSort(  f->formatName() );
	}
    }

    return result;
}

/*!
  Registers a new QImageFormatDecoderFactory.  This is not needed in
  application code as factories call this themselves.
*/
void QImageDecoder::registerDecoderFactory(QImageFormatDecoderFactory* f)
{
    QImageDecoderPrivate::factories.insert(0,f);
}

/*!
  Unregisters a new QImageFormatDecoderFactory.  This is not needed in
  application code as factories call this themselves.
*/
void QImageDecoder::unregisterDecoderFactory(QImageFormatDecoderFactory* f)
{
    QImageDecoderPrivate::factories.remove(f);
}

/*!
  \class QImageFormatDecoder qasyncimageio.h
  \brief Incremental image decoder for a specific image format.

  \internal
*/

/*!
  \fn int QImageFormatDecoder::decode(QImage& img, QImageConsumer* consumer,
	    const uchar* buffer, int length)

  Image decoders for specific image formats must override this method.
  It should decode some or all of the bytes in the given buffer into the
  given image, calling the given consumer as the decoding proceeds.
  The consumer may be 0.
*/

/*!
  \class QImageFormatDecoderFactory qasyncimageio.h
  \brief Factory that makes QImageFormatDecoder objects.

  \internal

  New image file formats are installed by creating objects of derived
  classes of QImageFormatDecoderFactory.  They must implement decoderFor()
  and formatName().
*/

/*!
  \fn virtual QImageFormatDecoder* QImageFormatDecoderFactory::decoderFor(const
	    uchar* buffer, int length)

  Returns a decoder for decoding an image which starts with the give bytes.
  This function should only return a decoder if it is definate that the
  decoder applies to data with the given header.  Returns 0 if there is
  insufficient data in the header to make a positive identification,
  or if the data is not recognized.
*/

/*!
  \fn virtual const char* QImageFormatDecoderFactory::formatName() const

  Returns the name of the format supported by decoders from this factory.
  The string is statically allocated.
*/

/*!
  Creates a factory.  It automatically registers itself with QImageDecoder.
*/
QImageFormatDecoderFactory::QImageFormatDecoderFactory()
{
    QImageDecoder::registerDecoderFactory(this);
}

/*!
  Destroys a factory.  It automatically unregisters itself from QImageDecoder.
*/
QImageFormatDecoderFactory::~QImageFormatDecoderFactory()
{
    QImageDecoder::unregisterDecoderFactory(this);
}

/*!
  \class QGIFDecoder qasyncimageio.h
  \brief Incremental image decoder for GIF image format.

  \internal
*/

/*!
  Constructs a QGIFDecoder.
*/
QGIFDecoder::QGIFDecoder()
{
    globalcmap_hold = 0;
    disposal = NoDisposal;
    disposed = TRUE;
    frame = -1;
    state = Header;
    count = 0;
    lcmap = FALSE;
}

/*!
  Destructs a QGIFDecoder.
*/
QGIFDecoder::~QGIFDecoder()
{
    delete globalcmap_hold;
}


class QGIFDecoderFactory : public QImageFormatDecoderFactory {
    QImageFormatDecoder* decoderFor(const uchar* buffer, int length);
    const char* formatName() const;
};
static QGIFDecoderFactory factory;

QImageFormatDecoder* QGIFDecoderFactory::decoderFor(
    const uchar* buffer, int length)
{
    if (length < 6) return 0;
    if (buffer[0]=='G'
     && buffer[1]=='I'
     && buffer[2]=='F'
     && buffer[3]=='8'
     && (buffer[4]=='9' || buffer[4]=='7')
     && buffer[5]=='a')
        return new QGIFDecoder;
    return 0;
}

const char* QGIFDecoderFactory::formatName() const
{
    return "GIF";
}



void QGIFDecoder::disposePrevious( QImage& img, QImageConsumer* consumer )
{
    // Handle disposal of previous image before processing next one

    if ( disposed ) return;

    switch (disposal) {
      case NoDisposal:
	break;
      case DoNotChange:
	break;
      case RestoreBackground:
	preserve_trans = FALSE;
	if (trans>=0) {
	    // Easy:  we use the transparent colour
	    fillRect(img, left, top, right-left+1, bottom-top+1, trans);
	} else if (bgcol>=0) {
	    // Easy:  we use the bgcol given
	    fillRect(img, left, top, right-left+1, bottom-top+1, bgcol);
	} else {
	    // Impossible:  We don't know of a bgcol - use pixel 0
	    uchar** line = img.jumpTable();
	    fillRect(img, left, top, right-left+1, bottom-top+1, line[0][0]);
	}
	if (consumer) {
	    bool d = !consumer->changed(QRect(left, top,
					      right-left+1, bottom-top+1));
	    digress = digress || d;
	}
	break;
      case RestoreImage: {
	uchar** line = img.jumpTable();
	preserve_trans = FALSE;
	for (int ln=top; ln<=bottom; ln++) {
	    memcpy(line[ln]+left,
		backingstore.scanLine(ln-top),
		right-left+1);
	}
	if (consumer) {
	    bool d = !consumer->changed(QRect(left, top,
					      right-left+1, bottom-top+1));
	    digress = digress || d;
	}
      }
    }
    disposal = NoDisposal; // Until an extension says otherwise.

    disposed = TRUE;
}

/*!
  This function decodes some data into image changes.

  Returns the number of bytes consumed.
*/
int QGIFDecoder::decode(QImage& img, QImageConsumer* consumer,
	const uchar* buffer, int length)
{
    // We are required to state that
    //    "The Graphics Interchange Format(c) is the Copyright property of
    //    CompuServe Incorporated. GIF(sm) is a Service Mark property of
    //    CompuServe Incorporated."

    #define LM(l, m) ((m)<<8|l)
    digress = FALSE;
    int initial = length;
    uchar** line = img.jumpTable();
    while (!digress && length) {
	length--;
	unsigned char ch=*buffer++;
	switch (state) {
	  case Header:
	    hold[count++]=ch;
	    if (count==6) {
		// Header
		gif89=(hold[3]!='8' || hold[4]!='7');
		state=LogicalScreenDescriptor;
		count=0;
	    }
	    break;
	  case LogicalScreenDescriptor:
	    hold[count++]=ch;
	    if (count==7) {
		// Logical Screen Descriptor
		swidth=LM(hold[0], hold[1]);
		sheight=LM(hold[2], hold[3]);
		gcmap=!!(hold[4]&0x80);
		//UNUSED: bpchan=(((hold[4]&0x70)>>3)+1);
		//UNUSED: gcmsortflag=!!(hold[4]&0x08);
		gncols=2<<(hold[4]&0x7);
		bgcol=(gcmap && hold[5]) ? hold[5] : -1;
		//aspect=hold[6] ? double(hold[6]+15)/64.0 : 1.0;

		img.create(swidth, sheight, 8, gcmap ? gncols : 256);
		line = img.jumpTable();
		if (consumer) digress = !consumer->setSize(swidth, sheight);

		trans = -1;
		preserve_trans = FALSE;
		count=0;
		ncols=gncols;
		if (gcmap) {
		    ccount=0;
		    state=GlobalColorMap;
		} else {
		    state=ImageDescriptor;
		}
	    }
	    break;
	  case GlobalColorMap: case LocalColorMap:
	    hold[count++]=ch;
	    if (count==3) {
		img.setColor(ccount,
		    (ccount==trans ? 0 : 0xff000000)
		    | qRgb(hold[0], hold[1], hold[2]));
		if (++ccount >= ncols) {
		    if ( state == LocalColorMap )
			state=TableImageLZWSize;
		    else
			state=Introducer;
		}
		count=0;
	    }
	    break;
	  case Introducer:
	    hold[count++]=ch;
	    switch (ch) {
	      case ',':
		state=ImageDescriptor;
		break;
	      case '!':
		state=ExtensionLabel;
		break;
	      case ';':
		if (consumer) digress = !consumer->end();
		state=Done;
		break;
	      default:
		digress=TRUE;
		// Unexpected Introducer - ignore block
		state=Error;
	    }
	    break;
	  case ImageDescriptor:
	    hold[count++]=ch;
	    if (count==10) {
		disposePrevious( img, consumer );
		disposed = FALSE;
		left=LM(hold[1], hold[2]);
		top=LM(hold[3], hold[4]);
		int width=LM(hold[5], hold[6]);
		int height=LM(hold[7], hold[8]);
		right=left+width-1;
		bottom=top+height-1;
		bool hadlcmap = lcmap;
		lcmap=!!(hold[9]&0x80);
		interlace=!!(hold[9]&0x40);
		//bool lcmsortflag=!!(hold[9]&0x20);
		int lncols=lcmap ? (2<<(hold[9]&0x7)) : 0;
		if (lncols) {
		    if (lncols > ncols) img.setNumColors(lncols);
		    ncols = lncols;
		} else {
		    ncols = gncols;
		}
		frame++;
		if ( frame == 0 ) {
		    if ( left || top || width!=swidth || height!=sheight ) {
			// Not full-size image - erase with bg or transparent
			if ( bgcol>=0 ) {
			    fillRect(img, 0, 0, swidth, sheight, bgcol);
			    if (consumer) digress = !consumer->changed(QRect(0,0,swidth,sheight));
			} else if ( trans > 0 ) {
			    fillRect(img, 0, 0, swidth, sheight, trans);
			    if (consumer) digress = !consumer->changed(QRect(0,0,swidth,sheight));
			}
		    }
		}

		if ( disposal == RestoreImage ) {
		    if (backingstore.width() < width
			|| backingstore.height() < height) {
			// We just use the backing store as a byte array
			backingstore.create( QMAX(backingstore.width(),
						  width),
					     QMAX(backingstore.height(),
						  height),
					     8,1);
		    }
		    for (int ln=0; ln<height; ln++) {
			memcpy(backingstore.scanLine(ln),
			       line[top+ln]+left, width);
		    }
		}

		count=0;
		if (lcmap) {
		    ccount=0;
		    state=LocalColorMap;
		    if (gcmap) {
			if (!globalcmap_hold) {
			    globalcmap_hold = new QRgb[256];
			    CHECK_PTR(globalcmap_hold);
			}
			memcpy(globalcmap_hold, img.colorTable(),
			    ncols * sizeof(QRgb));
		    }
		} else {
		    if (hadlcmap && gcmap) {
			memcpy(img.colorTable(), globalcmap_hold,
			    ncols * sizeof(QRgb));
		    }
		    state=TableImageLZWSize;
		}
		x = left;
		y = top;
		accum = 0;
		bitcount = 0;
		sp = stack;
		needfirst = FALSE;
	    }
	    break;
	  case TableImageLZWSize: {
	    lzwsize=ch;
	    if ( lzwsize > max_lzw_bits ) {
		state=Error;
	    } else {
		code_size=lzwsize+1;
		clear_code=1<<lzwsize;
		end_code=clear_code+1;
		max_code_size=2*clear_code;
		max_code=clear_code+2;
		int i;
		for (i=0; i<clear_code && i<(1<<max_lzw_bits); i++) {
		    table[0][i]=0;
		    table[1][i]=i;
		}
		for (i=clear_code; i<(1<<max_lzw_bits); i++) {
		    table[0][i]=table[1][i]=0;
		}
		state=ImageDataBlockSize;
	    }
	    count=0;
	    break;
	  } case ImageDataBlockSize:
	    expectcount=ch;
	    if (expectcount) {
		state=ImageDataBlock;
	    } else {
		if (consumer) digress = !consumer->frameDone();

		state=Introducer;
	    }
	    break;
	  case ImageDataBlock:
	    count++;
	    accum|=(ch<<bitcount);
	    bitcount+=8;
	    while (bitcount>=code_size && state==ImageDataBlock) {
		int code=accum&((1<<code_size)-1);
		bitcount-=code_size;
		accum>>=code_size;

		if (code==clear_code) {
		    if (!needfirst) {
			int i;
			code_size=lzwsize+1;
			max_code_size=2*clear_code;
			max_code=clear_code+2;
			for (i=0; i<clear_code; i++) {
			    table[0][i]=0;
			    table[1][i]=i;
			}
			for (i=clear_code; i<(1<<max_lzw_bits); i++) {
			    table[0][i]=table[1][i]=0;
			}
		    }
		    needfirst=TRUE;
		} else if (code==end_code) {
		    bitcount = -32768;
		    // Left the block end arrive
		} else {
		    if (needfirst) {
			firstcode=oldcode=code;
			if (!(preserve_trans && firstcode==trans))
			    line[y][x] = firstcode;
			x++;
			needfirst=FALSE;
			if (x>right) {
			    x=left;
			    nextY(img,consumer);
			}
		    } else {
			incode=code;
			if (code>=max_code) {
			    *sp++=firstcode;
			    code=oldcode;
			}
			while (code>=clear_code) {
			    *sp++=table[1][code];
			    if (code==table[0][code]) {
				state=Error;
				break;
			    }
			    if (sp-stack>=(1<<(max_lzw_bits))*2) {
				state=Error;
				break;
			    }
			    code=table[0][code];
			}
			*sp++=firstcode=table[1][code];
			code=max_code;
			if (code<(1<<max_lzw_bits)) {
			    table[0][code]=oldcode;
			    table[1][code]=firstcode;
			    max_code++;
			    if ((max_code>=max_code_size)
			     && (max_code_size<(1<<max_lzw_bits)))
			    {
				max_code_size*=2;
				code_size++;
			    }
			}
			oldcode=incode;
			while (sp>stack) {
			    --sp;
			    if (!(preserve_trans && *sp==trans))
				line[y][x] = *sp;
			    x++;
			    if (x>right) {
				x=left;
				nextY(img,consumer);
			    }
			}
		    }
		}
	    }
	    if (count==expectcount) {
		count=0;
		state=ImageDataBlockSize;
	    }
	    break;
	  case ExtensionLabel:
	    switch (ch) {
	     case 0xf9:
		state=GraphicControlExtension;
		break;
	     case 0xff:
		state=ApplicationExtension;
		break;
/////////////////////////////////////////// Ignored at this time //////
//           case 0xfe:
//                state=CommentExtension;
//                break;
//           case 0x01:
//                break;
///////////////////////////////////////////////////////////////////////
	     default:
		state=SkipBlockSize;
	    }
	    count=0;
	    break;
	  case ApplicationExtension:
	    if (count<11) hold[count]=ch;
	    count++;
	    if (count==hold[0]+1) {
		if (strncmp((char*)(hold+1), "NETSCAPE", 8)==0) {
		    // Looping extension
		    state=NetscapeExtensionBlockSize;
		} else {
		    state=SkipBlockSize;
		}
		count=0;
	    }
	    break;
	  case NetscapeExtensionBlockSize:
	    expectcount=ch;
	    count=0;
	    if (expectcount) state=NetscapeExtensionBlock;
	    else state=Introducer;
	    break;
	  case NetscapeExtensionBlock:
	    if (count<3) hold[count]=ch;
	    count++;
	    if (count==expectcount) {
		int loop = hold[0]+hold[1]*256;

		// Why if the extension here, if it is supposed to only
		// play through once?  We assume that the creator meant
		// 0, which is infinite.
		if (loop == 1) loop = 0;

		if (consumer) consumer->setLooping(loop);
		state=SkipBlockSize; // Ignore further blocks
	    }
	    break;
	  case GraphicControlExtension:
	    if (count<5) hold[count]=ch;
	    count++;
	    if (count==hold[0]+1) {
		disposePrevious( img, consumer );
		disposal=Disposal(hold[1]>>2&0x7);
		//UNUSED: waitforuser=!!((hold[1]>>1)&0x1);
		int delay=count>3 ? LM(hold[2], hold[3]) : 0;
		bool havetrans=hold[1]&0x1;
		int newtrans=havetrans ? hold[4] : -1;
		if (newtrans >= ncols) {
		    // Ignore invalid transparency.
		    newtrans=-1;
		    havetrans=FALSE;
		}
		if (trans >= 0) preserve_trans = TRUE;
		if (newtrans != trans) {
		    // Unset old transparency
		    if (trans >= 0) {
			img.setColor(trans, 0xff000000|img.color(trans));
			if ( newtrans >= 0 ) {
			    // Changed transparency.  Groan.
			    // Change all occurrence of old to new.
			    uchar** line = img.jumpTable();
			    for (int j=0; j<sheight; j++) {
				for (int i=0; i<swidth; i++) {
				    if (line[j][i]==trans) {
					line[j][i]=newtrans;
				    }
				}
			    }
			}
		    }
		    trans = newtrans;
		    if (trans >= 0) {
			if (globalcmap_hold) {
			    globalcmap_hold[trans]&=0x00ffffff;
			}
			img.setColor(trans, 0x00ffffff&img.color(trans));
		    }
		}
		img.setAlphaBuffer(havetrans);
		if (consumer) consumer->setFramePeriod(delay*10);
		count=0;
		state=SkipBlockSize;
	    }
	    break;
	  case SkipBlockSize:
	    expectcount=ch;
	    count=0;
	    if (expectcount) state=SkipBlock;
	    else state=Introducer;
	    break;
	  case SkipBlock:
	    count++;
	    if (count==expectcount) state=SkipBlockSize;
	    break;
	  case Done:
	    length++; // Unget
	    digress=TRUE;
	    state=Error; // More calls to this is an error
	    break;
	  case Error:
	    return -1; // Called again after done.
	}
    }
    return initial-length;
}

void QGIFDecoder::fillRect(QImage& img, int col, int row, int w, int h, uchar color)
{
    if (w>0) {
	uchar** line = img.jumpTable() + row;
	for (int j=0; j<h; j++) {
	    memset(line[j]+col, color, w);
	}
    }
}

void QGIFDecoder::nextY(QImage& img, QImageConsumer* consumer)
{
    int my;
    switch (interlace) {
      case 0:
	// Non-interlaced
	if (consumer) digress =
	    !consumer->changed(QRect(left, y, right-left+1, 1));
	y++;
	break;
      case 1:
	{
	    int i;
	    my = QMIN(7, bottom-y);
	    for (i=1; i<=my; i++)
	        memcpy(img.scanLine(y+i), img.scanLine(y), img.width());
	    if (consumer) digress =
	        !consumer->changed(QRect(left, y, right-left+1, my+1));
	    y+=8;
	    if (y>bottom) { interlace++; y=4; }
	} break;
      case 2:
	{
	    int i;
	    my = QMIN(3, bottom-y);
	    for (i=1; i<=my; i++)
	        memcpy(img.scanLine(y+i), img.scanLine(y), img.width());
    	    if (consumer) digress =
	        !consumer->changed(QRect(left, y, right-left+1, my+1));
	    y+=8;
	    if (y>bottom) { interlace++; y=2; }
	} break;
      case 3:
	{
	    my = QMIN(1, bottom-y);
	    for (int i=1; i<=my; i++)
	        memcpy(img.scanLine(y+i), img.scanLine(y), img.width());
	    if (consumer) digress =
	        !consumer->changed(QRect(left, y, right-left+1, my+1));
	    y+=4;
	    if (y>bottom) { interlace++; y=1; }
	} break;
      case 4:
	if (consumer) digress =
	    !consumer->changed(QRect(left, y, right-left+1, 1));
	y+=2;
    }

    // Consume bogus extra lines
    if (y > bottom) y=bottom;
}

