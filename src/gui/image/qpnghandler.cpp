/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "private/qpnghandler_p.h"

#include <qiodevice.h>
#include <qimage.h>
#include <qlist.h>
#include <qcoreapplication.h>
#include <qvariant.h>
#include <qvector.h>

#include <png.h>

#ifdef Q_OS_TEMP
#define CALLBACK_CALL_TYPE        __cdecl
#else
#define CALLBACK_CALL_TYPE
#endif


/*
  All PNG files load to the minimal QImage equivalent.

  All QImage formats output to reasonably efficient PNG equivalents.
  Never to grayscale.
*/

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

class QPNGImageWriter {
public:
    explicit QPNGImageWriter(QIODevice*);
    ~QPNGImageWriter();

    enum DisposalMethod { Unspecified, NoDisposal, RestoreBackground, RestoreImage };
    void setDisposalMethod(DisposalMethod);
    void setLooping(int loops=0); // 0 == infinity
    void setFrameDelay(int msecs);
    void setGamma(float);

    bool writeImage(const QImage& img, int x, int y);
    bool writeImage(const QImage& img, int quality, int x, int y);
    bool writeImage(const QImage& img)
        { return writeImage(img, 0, 0); }
    bool writeImage(const QImage& img, int quality)
        { return writeImage(img, quality, 0, 0); }

    QIODevice* device() { return dev; }

private:
    QIODevice* dev;
    int frames_written;
    DisposalMethod disposal;
    int looping;
    int ms_delay;
    float gamma;
};

static
void CALLBACK_CALL_TYPE iod_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    QIODevice *in = (QIODevice *)png_get_io_ptr(png_ptr);

    while (length) {
        int nr = in->read((char*)data, length);
        if (nr <= 0) {
            png_error(png_ptr, "Read Error");
            return;
        }
        length -= nr;
    }
}


static
void CALLBACK_CALL_TYPE qpiw_write_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    QPNGImageWriter* qpiw = (QPNGImageWriter*)png_get_io_ptr(png_ptr);
    QIODevice* out = qpiw->device();

    uint nr = out->write((char*)data, length);
    if (nr != length) {
        png_error(png_ptr, "Write Error");
        return;
    }
}


static
void CALLBACK_CALL_TYPE qpiw_flush_fn(png_structp /* png_ptr */)
{
}

#if defined(Q_C_CALLBACKS)
}
#endif

static
void setup_qt(QImage& image, png_structp png_ptr, png_infop info_ptr, float screen_gamma=0.0)
{
    if (screen_gamma != 0.0 && png_get_valid(png_ptr, info_ptr, PNG_INFO_gAMA)) {
        double file_gamma;
        png_get_gAMA(png_ptr, info_ptr, &file_gamma);
        png_set_gamma(png_ptr, screen_gamma, file_gamma);
    }

    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);

    if (color_type == PNG_COLOR_TYPE_GRAY) {
        // Black & White or 8-bit grayscale
        if (bit_depth == 1 && info_ptr->channels == 1) {
            png_set_invert_mono(png_ptr);
            png_read_update_info(png_ptr, info_ptr);
            image = QImage(width, height, QImage::Format_Mono);
            if (image.isNull())
                return;
            image.setNumColors(2);
            image.setColor(1, qRgb(0,0,0));
            image.setColor(0, qRgb(255,255,255));
        } else if (bit_depth == 16 && png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            png_set_expand(png_ptr);
            png_set_strip_16(png_ptr);
            png_set_gray_to_rgb(png_ptr);
            image = QImage(width, height, QImage::Format_ARGB32);
            if (image.isNull())
                return;
            if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
                png_set_swap_alpha(png_ptr);

            png_read_update_info(png_ptr, info_ptr);
        } else {
            if (bit_depth == 16)
                png_set_strip_16(png_ptr);
            else if (bit_depth < 8)
                png_set_packing(png_ptr);
            int ncols = bit_depth < 8 ? 1 << bit_depth : 256;
            png_read_update_info(png_ptr, info_ptr);
            image = QImage(width, height, QImage::Format_Indexed8);
            if (image.isNull())
                return;
            image.setNumColors(ncols);
            for (int i=0; i<ncols; i++) {
                int c = i*255/(ncols-1);
                image.setColor(i, qRgba(c,c,c,0xff));
            }
            if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
                const int g = info_ptr->trans_values.gray;
                if (g < ncols) {
                    image.setColor(g, 0);
                }
            }
        }
    } else if (color_type == PNG_COLOR_TYPE_PALETTE
               && png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE)
               && info_ptr->num_palette <= 256)
    {
        // 1-bit and 8-bit color
        if (bit_depth != 1)
            png_set_packing(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);
        image = QImage(width, height, bit_depth == 1 ? QImage::Format_Mono : QImage::Format_Indexed8);
        if (image.isNull())
            return;
        image.setNumColors(info_ptr->num_palette);
        int i = 0;
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            while (i < info_ptr->num_trans) {
                image.setColor(i, qRgba(
                    info_ptr->palette[i].red,
                    info_ptr->palette[i].green,
                    info_ptr->palette[i].blue,
                    info_ptr->trans[i]
                   )
               );
                i++;
            }
        }
        while (i < info_ptr->num_palette) {
            image.setColor(i, qRgba(
                info_ptr->palette[i].red,
                info_ptr->palette[i].green,
                info_ptr->palette[i].blue,
                0xff
               )
           );
            i++;
        }
    } else {
        // 32-bit
        if (bit_depth == 16)
            png_set_strip_16(png_ptr);

        png_set_expand(png_ptr);

        if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png_ptr);

        QImage::Format format = QImage::Format_ARGB32;
        // Only add filler if no alpha, or we can get 5 channel data.
        if (!(color_type & PNG_COLOR_MASK_ALPHA)
            && !png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            png_set_filler(png_ptr, 0xff, QSysInfo::ByteOrder == QSysInfo::BigEndian ?
                           PNG_FILLER_BEFORE : PNG_FILLER_AFTER);
            // We want 4 bytes, but it isn't an alpha channel
            format = QImage::Format_RGB32;
        }
        image = QImage(width, height, format);
        if (image.isNull())
            return;

        if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
            png_set_swap_alpha(png_ptr);

        png_read_update_info(png_ptr, info_ptr);
    }

    // Qt==ARGB==Big(ARGB)==Little(BGRA)
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        png_set_bgr(png_ptr);
    }
}


#if defined(Q_C_CALLBACKS)
extern "C" {
#endif
static void CALLBACK_CALL_TYPE qt_png_warning(png_structp /*png_ptr*/, png_const_charp message)
{
    qWarning("libpng warning: %s", message);
}

#if defined(Q_C_CALLBACKS)
}
#endif


static bool read_png_image(QIODevice *device, QImage *outImage, float gamma)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;
    png_bytep* row_pointers;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
    if (!png_ptr)
        return false;

    png_set_error_fn(png_ptr, 0, 0, qt_png_warning);

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, 0, 0);
        return false;
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        return false;
    }

    if (setjmp(png_ptr->jmpbuf)) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return false;
    }

    png_set_read_fn(png_ptr, device, iod_read_fn);
    png_read_info(png_ptr, info_ptr);

    QImage image;
    setup_qt(image, png_ptr, info_ptr, gamma);
    if (image.isNull()) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return false;
    }

    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
        0, 0, 0);

    uchar *data = image.bits();
    int bpl = image.bytesPerLine();
    row_pointers=new png_bytep[height];

    for (uint y=0; y<height; y++) {
        row_pointers[y] = data + y * bpl;
    }

    png_read_image(png_ptr, row_pointers);

#if 0 // libpng takes care of this.
    png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)
    if (image.depth()==32 && png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        QRgb trans = 0xFF000000 | qRgb(
              (info_ptr->trans_values.red << 8 >> bit_depth)&0xff,
              (info_ptr->trans_values.green << 8 >> bit_depth)&0xff,
              (info_ptr->trans_values.blue << 8 >> bit_depth)&0xff);
        for (uint y=0; y<height; y++) {
            for (uint x=0; x<info_ptr->width; x++) {
                if (((uint**)jt)[y][x] == trans) {
                    ((uint**)jt)[y][x] &= 0x00FFFFFF;
                } else {
                }
            }
        }
    }
#endif

    image.setDotsPerMeterX(png_get_x_pixels_per_meter(png_ptr,info_ptr));
    image.setDotsPerMeterY(png_get_y_pixels_per_meter(png_ptr,info_ptr));

#ifndef QT_NO_IMAGE_TEXT
    png_textp text_ptr;
    int num_text=0;
    png_get_text(png_ptr,info_ptr,&text_ptr,&num_text);
    while (num_text--) {
        image.setText(text_ptr->key,0,text_ptr->text);
        text_ptr++;
    }
#endif

    delete [] row_pointers;

    *outImage = image;

    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    return true;
}

QPNGImageWriter::QPNGImageWriter(QIODevice* iod) :
    dev(iod),
    frames_written(0),
    disposal(Unspecified),
    looping(-1),
    ms_delay(-1),
    gamma(0.0)
{
}

QPNGImageWriter::~QPNGImageWriter()
{
}

void QPNGImageWriter::setDisposalMethod(DisposalMethod dm)
{
    disposal = dm;
}

void QPNGImageWriter::setLooping(int loops)
{
    looping = loops;
}

void QPNGImageWriter::setFrameDelay(int msecs)
{
    ms_delay = msecs;
}

void QPNGImageWriter::setGamma(float g)
{
    gamma = g;
}


#ifndef QT_NO_IMAGE_TEXT
static void set_text(const QImage& image, png_structp png_ptr, png_infop info_ptr, bool short_not_long)
{
    QList<QImageTextKeyLang> keys = image.textList();
    if (keys.count()) {
        png_textp text_ptr = new png_text[keys.count()];
        int i=0;
        QList<QByteArray> tlist;
        for (QList<QImageTextKeyLang>::Iterator it=keys.begin();
                it != keys.end(); ++it)
        {
            QString t = image.text(*it);
            if ((t.length() <= 200) == short_not_long) {
                if (t.length() < 40)
                    text_ptr[i].compression = PNG_TEXT_COMPRESSION_NONE;
                else
                    text_ptr[i].compression = PNG_TEXT_COMPRESSION_zTXt;
                text_ptr[i].key = (png_charp)(*it).key.data();
                tlist.append(t.toLatin1());
                text_ptr[i].text = (png_charp)tlist.last().constData();
                //text_ptr[i].text = qstrdup(t.latin1());
                i++;
            }
        }
        png_set_text(png_ptr, info_ptr, text_ptr, i);
        //for (int j=0; j<i; j++)
            //free(text_ptr[i].text);
        delete [] text_ptr;
    }
}
#endif

bool QPNGImageWriter::writeImage(const QImage& image, int off_x, int off_y)
{
    return writeImage(image, -1, off_x, off_y);
}

bool QPNGImageWriter::writeImage(const QImage& image, int quality_in, int off_x_in, int off_y_in)
{
    QPoint offset = image.offset();
    int off_x = off_x_in + offset.x();
    int off_y = off_y_in + offset.y();

    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep* row_pointers;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
    if (!png_ptr) {
        return false;
    }

    png_set_error_fn(png_ptr, 0, 0, qt_png_warning);

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, 0);
        return false;
    }

    if (setjmp(png_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }

    int quality = quality_in;
    if (quality >= 0) {
        if (quality > 9) {
            qWarning("PNG: Quality %d out of range", quality);
            quality = 9;
        }
        png_set_compression_level(png_ptr, quality);
    }

    if (gamma != 0.0) {
        png_set_gAMA(png_ptr, info_ptr, 1.0/gamma);
    }

    png_set_write_fn(png_ptr, (void*)this, qpiw_write_fn, qpiw_flush_fn);

    info_ptr->channels =
        (image.depth() == 32)
        ? (image.format() == QImage::Format_RGB32 ? 3 : 4)
        : 1;

    png_set_IHDR(png_ptr, info_ptr, image.width(), image.height(),
        image.depth() == 1 ? 1 : 8 /* per channel */,
        image.depth() == 32
            ? image.format() == QImage::Format_RGB32
                ? PNG_COLOR_TYPE_RGB
                : PNG_COLOR_TYPE_RGB_ALPHA
            : PNG_COLOR_TYPE_PALETTE, 0, 0, 0);


    //png_set_sBIT(png_ptr, info_ptr, 8);
    info_ptr->sig_bit.red = 8;
    info_ptr->sig_bit.green = 8;
    info_ptr->sig_bit.blue = 8;

    if (image.format() == QImage::Format_MonoLSB)
       png_set_packswap(png_ptr);

    png_colorp palette = 0;
    png_bytep copy_trans = 0;
    if (image.numColors()) {
        // Paletted
        int num_palette = image.numColors();
        palette = new png_color[num_palette];
        png_set_PLTE(png_ptr, info_ptr, palette, num_palette);
        int* trans = new int[num_palette];
        int num_trans = 0;
        for (int i=0; i<num_palette; i++) {
            QRgb rgb=image.color(i);
            info_ptr->palette[i].red = qRed(rgb);
            info_ptr->palette[i].green = qGreen(rgb);
            info_ptr->palette[i].blue = qBlue(rgb);
            trans[i] = rgb >> 24;
            if (trans[i] < 255) {
                num_trans = i+1;
            }
        }
        if (num_trans) {
            copy_trans = new png_byte[num_trans];
            for (int i=0; i<num_trans; i++)
                copy_trans[i] = trans[i];
            png_set_tRNS(png_ptr, info_ptr, copy_trans, num_trans, 0);
        }
        delete [] trans;
    }

    if (image.format() != QImage::Format_RGB32) {
        info_ptr->sig_bit.alpha = 8;
    }

    // Swap ARGB to RGBA (normal PNG format) before saving on
    // BigEndian machines
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        png_set_swap_alpha(png_ptr);
    }

    // Qt==ARGB==Big(ARGB)==Little(BGRA)
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        png_set_bgr(png_ptr);
    }

    if (off_x || off_y) {
        png_set_oFFs(png_ptr, info_ptr, off_x, off_y, PNG_OFFSET_PIXEL);
    }

    if (frames_written > 0)
        png_set_sig_bytes(png_ptr, 8);

    if (image.dotsPerMeterX() > 0 || image.dotsPerMeterY() > 0) {
        png_set_pHYs(png_ptr, info_ptr,
                image.dotsPerMeterX(), image.dotsPerMeterY(),
                PNG_RESOLUTION_METER);
    }

#ifndef QT_NO_IMAGE_TEXT
    // Write short texts early.
    set_text(image,png_ptr,info_ptr,true);
#endif

    png_write_info(png_ptr, info_ptr);

#ifndef QT_NO_IMAGE_TEXT
    // Write long texts later.
    set_text(image,png_ptr,info_ptr,false);
#endif

    if (image.depth() != 1)
        png_set_packing(png_ptr);

    if (image.format() == QImage::Format_RGB32)
        png_set_filler(png_ptr, 0,
            QSysInfo::ByteOrder == QSysInfo::BigEndian ?
                PNG_FILLER_BEFORE : PNG_FILLER_AFTER);

    if (looping >= 0 && frames_written == 0) {
        uchar data[13] = "NETSCAPE2.0";
        //                0123456789aBC
        data[0xB] = looping%0x100;
        data[0xC] = looping/0x100;
        png_write_chunk(png_ptr, (png_byte*)"gIFx", data, 13);
    }
    if (ms_delay >= 0 || disposal!=Unspecified) {
        uchar data[4];
        data[0] = disposal;
        data[1] = 0;
        data[2] = (ms_delay/10)/0x100; // hundredths
        data[3] = (ms_delay/10)%0x100;
        png_write_chunk(png_ptr, (png_byte*)"gIFg", data, 4);
    }

    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
        0, 0, 0);

    const uchar *data = image.bits();
    int bpl = image.bytesPerLine();
    row_pointers = new png_bytep[height];
    uint y;
    for (y=0; y<height; y++) {
        row_pointers[y] = (png_bytep)(data + y * bpl);
    }
    png_write_image(png_ptr, row_pointers);
    delete [] row_pointers;

    png_write_end(png_ptr, info_ptr);
    frames_written++;

    if (palette)
        delete [] palette;
    if (copy_trans)
        delete [] copy_trans;

    png_destroy_write_struct(&png_ptr, &info_ptr);

    return true;
}

static bool write_png_image(const QImage &image, QIODevice *device, int quality, float gamma)
{
    QPNGImageWriter writer(device);
    if (quality >= 0) {
        quality = qMin(quality, 100);
        quality = (100-quality) * 9 / 91; // map [0,100] -> [9,0]
    }
    writer.setGamma(gamma);
    return writer.writeImage(image, quality);
}

QPngHandler::QPngHandler()
{
    gamma = 0.0;
    quality = 2;
}

bool QPngHandler::canRead() const
{
    return canRead(device());
}

bool QPngHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("QPngHandler::canRead() called with no device");
        return false;
    }

    qint64 oldPos = device->pos();

    char head[4];
    qint64 readBytes = device->read(head, sizeof(head));
    if (readBytes != sizeof(head)) {
        if (device->isSequential()) {
            while (readBytes > 0)
                device->ungetChar(head[readBytes-- - 1]);
        } else {
            device->seek(oldPos);
        }
        return false;
    }

    if (device->isSequential()) {
        while (readBytes > 0)
            device->ungetChar(head[readBytes-- - 1]);
    } else {
        device->seek(oldPos);
    }

    return qstrncmp(head + 1, "PNG", 3) == 0;
}

bool QPngHandler::read(QImage *image)
{
    return read_png_image(device(), image, gamma);
}

bool QPngHandler::write(const QImage &image)
{
    return write_png_image(image, device(), quality, gamma);
}

bool QPngHandler::supportsOption(ImageOption option) const
{
    return option == Gamma || option == Quality;
}

QVariant QPngHandler::option(ImageOption option) const
{
    if (option == Gamma)
        return gamma;
    else if (option == Quality)
        return quality;
    return 0;
}

void QPngHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == Gamma)
        gamma = value.toDouble();
    else if (option == Quality)
        quality = value.toInt();
}

QByteArray QPngHandler::name() const
{
    return "png";
}
