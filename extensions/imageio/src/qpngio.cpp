/****************************************************************************
** $Id: //depot/qt/main/extensions/imageio/src/qpngio.cpp#1 $
**
** Implementation of PNG QImage IOHandler
**
** Created : 970521
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

extern "C" {
#include <png.h>
}
#include <qimage.h>
#include <qiodev.h>

static
void iod_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    QImageIO* iio = (QImageIO*)png_get_io_ptr(png_ptr);
    QIODevice* in = iio->ioDevice();

    while (length) {
	int nr = in->readBlock((char*)data, length);
	if (nr <= 0) {
	    png_error(png_ptr, "Read Error");
	    return;
	}
	length -= nr;
    }
}

static
void iod_write_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    QImageIO* iio = (QImageIO*)png_get_io_ptr(png_ptr);
    QIODevice* out = iio->ioDevice();

    uint nr = out->writeBlock((char*)data, length);
    if (nr != length) {
	png_error(png_ptr, "Write Error");
	return;
    }
}

static
void read_png_image(QImageIO* iio)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;
    png_bytep* row_pointers;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
    if (!png_ptr) {
	iio->setStatus(-1);
	return;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
	png_destroy_read_struct(&png_ptr, 0, 0);
	iio->setStatus(-2);
	return;
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	iio->setStatus(-3);
	return;
    }

    if (setjmp(png_ptr->jmpbuf)) {
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	iio->setStatus(-4);
	return;
    }

    png_set_read_fn(png_ptr, (void*)iio, iod_read_fn);


    png_read_info(png_ptr, info_ptr);
    png_set_strip_16(png_ptr);

    if (info_ptr->bit_depth < 8
    && (info_ptr->bit_depth != 1
     || info_ptr->channels != 1
     || (info_ptr->color_type != PNG_COLOR_TYPE_GRAY
      && info_ptr->color_type != PNG_COLOR_TYPE_PALETTE)))
    {
	png_set_packing(png_ptr);
    }

    if (info_ptr->valid & PNG_INFO_gAMA)
	png_set_gamma(png_ptr, 2.2, info_ptr->gamma);

    QImage image;

    if (info_ptr->bit_depth == 1
     && info_ptr->channels == 1
     && info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
    {
	// Black & White
	png_set_invert_mono(png_ptr);
	png_read_update_info(png_ptr, info_ptr);
	image.create(info_ptr->width,info_ptr->height, 1, 2,
	    QImage::BigEndian);
	image.setColor(1, qRgb(0,0,0) );
	image.setColor(0, qRgb(255,255,255) );
    } else if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE
    && (info_ptr->valid & PNG_INFO_PLTE)
    && (!(info_ptr->valid & PNG_INFO_tRNS))
    && info_ptr->num_palette <= 256)
    {
	// 1-bit and 8-bit color
	png_read_update_info(png_ptr, info_ptr);
	image.create(
	    info_ptr->width,
	    info_ptr->height,
	    info_ptr->bit_depth,
	    info_ptr->num_palette,
	    QImage::BigEndian
	);
	for (int i=0; i<info_ptr->num_palette; i++) {
	    image.setColor(i, qRgb(
		info_ptr->palette[i].red,
		info_ptr->palette[i].green,
		info_ptr->palette[i].blue
		)
	    );
	}
    } else if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
    {
	// 8-bit greyscale
	int ncols = info_ptr->bit_depth < 8 ? 1 << info_ptr->bit_depth : 256;
	png_read_update_info(png_ptr, info_ptr);
	image.create(info_ptr->width,info_ptr->height,8,ncols);
	for (int i=0; i<ncols; i++) {
	    int c = i*255/(ncols-1);
	    image.setColor( i, qRgb(c,c,c) );
	}
    } else {
	// 32-bit
	png_set_expand(png_ptr);
	
	if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY ||
	    info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
	    png_set_gray_to_rgb(png_ptr);
	}

	// Only add filler if no alpha, or we'll get 5 channel data!
	if (!(info_ptr->color_type & PNG_COLOR_MASK_ALPHA))
	    png_set_filler(png_ptr, 0xff,
		QImage::systemByteOrder() == QImage::BigEndian ?
		    PNG_FILLER_BEFORE : PNG_FILLER_AFTER);

	png_read_update_info(png_ptr, info_ptr);
	image.create(info_ptr->width,info_ptr->height,32);
    }

    if (info_ptr->channels == 4) {
	image.setAlphaBuffer(TRUE);
    } else {
	image.setAlphaBuffer(FALSE);
    }

    uchar** jt = image.jumpTable();
    row_pointers=new png_bytep[info_ptr->height];
    {
	    uint y;
	    for (y=0; y<info_ptr->height; y++) {
		    row_pointers[y]=jt[y];
	    }
    }
    png_read_image(png_ptr, row_pointers);
    delete row_pointers;

    iio->setImage(image);

    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    iio->setStatus(0);
}

static
void write_png_image(QImageIO* iio)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep* row_pointers;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
    if (!png_ptr) {
	iio->setStatus(-1);
	return;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
	png_destroy_write_struct(&png_ptr, 0);
	iio->setStatus(-2);
	return;
    }

    if (setjmp(png_ptr->jmpbuf)) {
	png_destroy_write_struct(&png_ptr, &info_ptr);
	iio->setStatus(-4);
	return;
    }

    png_set_write_fn(png_ptr, (void*)iio, iod_write_fn, 0);

    const QImage& image = iio->image();

    info_ptr->width = image.width();
    info_ptr->height = image.height();

    switch (image.depth()) {
      case 1:
	info_ptr->bit_depth = 1;
	info_ptr->color_type = PNG_COLOR_TYPE_PALETTE;
	break;
      case 8:
	info_ptr->bit_depth = 8;
	info_ptr->color_type = PNG_COLOR_TYPE_PALETTE;
	break;
      case 32:
	info_ptr->bit_depth = 8; // per channel
	info_ptr->color_type = image.hasAlphaBuffer()
	    ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB;
	break;
    }

    info_ptr->sig_bit.red = 8;
    info_ptr->sig_bit.green = 8;
    info_ptr->sig_bit.blue = 8;

    if (image.numColors()) {
	// Paletted
	info_ptr->valid |= PNG_INFO_PLTE;
	info_ptr->palette = new png_color[image.numColors()];
	info_ptr->num_palette = image.numColors();
	int trans[info_ptr->num_palette];
	int num_trans = 0;
	for (int i=0; i<info_ptr->num_palette; i++) {
	    QRgb rgb=image.color(i);
	    info_ptr->palette[i].red = qRed(rgb);
	    info_ptr->palette[i].green = qGreen(rgb);
	    info_ptr->palette[i].blue = qBlue(rgb);
	    if (image.hasAlphaBuffer()) {
		trans[i] = rgb >> 24;
		if (trans[i] < 255) {
		    num_trans = i+1;
		}
	    }
	}
	if (num_trans) {
	    info_ptr->valid |= PNG_INFO_tRNS;
	    info_ptr->trans = new png_byte[num_trans];
	    info_ptr->num_trans = num_trans;
	    for (int i=0; i<num_trans; i++)
		info_ptr->trans[i] = trans[i];
	}
    }

    if ( image.hasAlphaBuffer() ) {
	info_ptr->sig_bit.alpha = 8;
    }

    png_write_info(png_ptr, info_ptr);

    png_set_packing(png_ptr);
    png_set_filler(png_ptr, 0,
	QImage::systemByteOrder() == QImage::BigEndian ?
	    PNG_FILLER_BEFORE : PNG_FILLER_AFTER);

    uchar** jt = image.jumpTable();
    row_pointers=new png_bytep[info_ptr->height];
    uint y;
    for (y=0; y<info_ptr->height; y++) {
	    row_pointers[y]=jt[y];
    }
    png_write_image(png_ptr, row_pointers);
    delete row_pointers;

    png_write_end(png_ptr, info_ptr);

    if (image.numColors())
	delete info_ptr->palette;
    if (info_ptr->valid & PNG_INFO_tRNS)
	delete info_ptr->trans;

    png_destroy_write_struct(&png_ptr, &info_ptr);

    iio->setStatus(0);
}


void qInitPngIO()
{
    QImageIO::defineIOHandler("PNG", "^.PNG\r", 0, read_png_image, write_png_image);
}
