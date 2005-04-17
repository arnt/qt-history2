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

#include <qfileinfo.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <qdom.h>
#include <qlocale.h>
#include <qtemporaryfile.h>

// Any changes here must be reflected in qresource.cpp
enum {
    Compressed = 0x01
};

static bool verbose = false;
static int compress_level = -1;
static int compress_threshold = 70;
static QString resource_root;

struct RCCFileInfo {
    QString name;
    QFileInfo fileinfo;
};
struct RCCResource {
    inline RCCResource() : lang(QLocale::C) {}
    QLocale lang;
    QString prefix;
    QList<RCCFileInfo> files;
};
QList<RCCResource>
listResourceFile(const QString &file)
{
    QList<RCCResource> ret;
    QFile in(file);
    if(!in.open(QIODevice::ReadOnly)) {
        fprintf(stderr, "Unable to open %s\n", file.toLatin1().constData());
        return ret;
    }
    QString filePath = QFileInfo(file).path();
    if(!filePath.isEmpty() && !filePath.endsWith(QLatin1String("/")))
        filePath += '/';
    QDomDocument document;
    document.setContent(&in);
    QDomElement root = document.firstChild().toElement();
    if(root.tagName() != QLatin1String("RCC")) {
#if 0
        RCCResource resource;
        RCCFileInfo resource_file;
        resource_file.fileinfo = QFileInfo(filePath + file);
        resource_file.name = resource_file.fileinfo.filePath();
        resource.files.append(resource_file);
        ret << resource;
#else
        fprintf(stderr, "Unable to parse %s\n", file.toLatin1().constData());
#endif
        return ret;
    }
    for(QDomElement child = root.firstChild().toElement(); !child.isNull();
        child = child.nextSibling().toElement()) {
        if(child.tagName() == QLatin1String("qresource")) {
            RCCResource resource;
            if(child.hasAttribute("lang"))
                resource.lang = QLocale(child.attribute("lang"));
            resource.prefix = child.attribute("prefix");
            for(QDomNode res = child.firstChild(); !res.isNull(); res = res.nextSibling()) {
                if(res.toElement().tagName() == QLatin1String("file")) {
                    QString fileName(res.firstChild().toText().data());
                    QFileInfo file(filePath + fileName);
                    QString alias;
                    if(res.toElement().hasAttribute("alias"))
                        alias = res.toElement().attribute("alias");
                    if(!file.isFile()) {
                        bool recursive = false;
                        if(res.toElement().hasAttribute("recursive")) {
                            QString s = res.toElement().attribute("recursive");
                            recursive = s.toLower() == "true";
                        }
                        QDir dir;
                        if(file.isDir()) {
                            dir = QDir(file.filePath(), "*");
                        } else {
                            fileName = fileName.section('/', 0, -2);
                            dir = QDir(file.path(), file.fileName());
                            dir.setFilter(QDir::Filters(QDir::Files|QDir::AllDirs));
                        }
                        QFileInfoList subFiles = dir.entryInfoList();
                        for(int subFile = 0; subFile < subFiles.count(); subFile++) {
                            if(subFiles[subFile].fileName() == "."
                               || subFiles[subFile].fileName() == "..")
                                continue;
                            if(!subFiles[subFile].isDir()) {
                                RCCFileInfo res;
                                if(!alias.isNull())
                                    res.name = alias + "/";
                                else
                                    res.name = fileName + "/";
                                res.name += subFiles[subFile].fileName();
                                res.fileinfo = subFiles[subFile];
                                resource.files.append(res);
                            }
                            if(recursive) {
                                //do we want to support recursive?
                            }
                        }
                    } else {
                        RCCFileInfo res;
                        if(!alias.isNull()) {
                            res.name = alias;
                        } else {
                            res.name = QDir::cleanPath(fileName);
                            while(res.name.startsWith(QLatin1String("../")))
                                res.name = res.name.mid(3);
                        }
                        res.fileinfo = QFileInfo(file);
                        resource.files.append(res);
                    }
                }
            }
            ret.append(resource);
        }
    }
    return ret;
}

bool
processResourceFile(const QString &file, FILE *out_fd, QStringList *created)
{
    QList<RCCResource> resources = listResourceFile(file);
    for(int resource = 0; resource < resources.size(); ++resource) {
        const RCCResource &r = resources.at(resource);
        for(int file = 0; file < r.files.count(); file++) {
            QFile inputQFile(r.files[file].fileinfo.filePath());
            if (!inputQFile.open(QIODevice::ReadOnly)) {
                qWarning("Could not open file '%s'", inputQFile.fileName().toLatin1().constData());
                continue;
            }
            int compressRatio = 0;
            QByteArray input = inputQFile.readAll();
            if(compress_level && input.length() > 100) {
                QByteArray compress = qCompress((uchar *)input.data(), input.size(), compress_level);
                compressRatio = (int)(((float)input.size())/compress.size()*100);
                if(compressRatio >= compress_threshold)
                    input = compress;
                else
                    compressRatio = 0;
            }

            //header
            const QString location = QDir::cleanPath(resource_root + "/" +
                                                     r.prefix + "/" + r.files[file].name);
            if(verbose)
                fprintf(stderr, "Read file %s(@%s) [Compressed %d%%]\n", inputQFile.fileName().toLatin1().data(),
                        location.toLatin1().data(), compressRatio);

            QByteArray resource_name;
            {
                const QChar *data = location.unicode();
                for(int i = 0; i < location.length(); i++) {
                    if(!(data+i)->row() &&
                       ((data+i)->cell() >= 'A' && (data+i)->cell() <= 'Z') ||
                       ((data+i)->cell() >= '0' && (data+i)->cell() <= '9') ||
                       ((data+i)->cell() >= 'a' && (data+i)->cell() <= 'z') ||
                       (data+i)->cell() == '_') {
                        resource_name += (data+i)->cell();
                    } else {
                        if((data+i)->row()) {
                            resource_name += "__";
                            resource_name += QByteArray::number((data+i)->row());
                        }
                        resource_name += "__";
                        resource_name += QByteArray::number((data+i)->cell());
                    }
                }
                if(r.lang.language() != QLocale::C) {
                    resource_name += "__";
                    resource_name += r.lang.name();
                }
            }
            uchar flags = 0;
            if(compressRatio)
                flags |= Compressed;
            fprintf(out_fd, "\n");
            fprintf(out_fd, "//Generated from '%s'\n", inputQFile.fileName().toLatin1().constData());
            fprintf(out_fd, "static uchar %s[] = {\n", resource_name.constData());
            fprintf(out_fd, "\t0x12, 0x15, 0x19, 0x78, //header\n");
            fprintf(out_fd, "\t0x01, //version\n");
            fprintf(out_fd, "\t0x%02x, 0x%02x, //lang\n", (uchar)r.lang.language(), (uchar)r.lang.country());
            fprintf(out_fd, "\t0x%02x, //flags\n", flags);

            //name
            fprintf(out_fd, "\n\t//name");
            for(int i = 0; i < location.length(); i++) {
                if(!(i % 5))
                    fprintf(out_fd, "\n\t");
                QChar c = location[i];
                if(c == QDir::separator())
                    c = '/';
                fprintf(out_fd, "0x%02x, 0x%02x, ", c.row(), c.cell());
            }
            fprintf(out_fd, "\n\t0x00, 0x00, \n");

            //bits
            fprintf(out_fd, "\n\t//bits\n");
            uchar bytesNeeded = 0;
            const int input_length = input.length();
            for(int length = input_length; length > 0; length >>= 8)
                bytesNeeded++;
            fprintf(out_fd, "\t0x%02x, //bytes in len\n\t", bytesNeeded);
            for(int i = bytesNeeded; i; i--)
                fprintf(out_fd, "0x%02x, ", ((input_length >> ((i-1)*8)) & 0xFF));
            fprintf(out_fd, "//length");
            const char *data = input.constData();
            for(int i = 0; i < input_length; i++) {
                if(!(i % 10))
                    fprintf(out_fd, "\n\t");
                if(i != input_length-1)
                    fprintf(out_fd, "0x%02x, ", (uchar)*(data+i));
                else
                    fprintf(out_fd, "0x%02x, ", (uchar)*(data+i));
            }

            //footer
            fprintf(out_fd, "\n};\n");

            //QMetaResource
            fprintf(out_fd, "Q_GLOBAL_STATIC_WITH_ARGS(QMetaResource, resource_%s, (%s))\n",
                    resource_name.constData(), resource_name.constData());
            if(created) {
                QString rc = "resource_" + resource_name;
#if 0
                if(created->contains(rc))
                    fprintf(stderr, "Warning: duplicate symbol %s[%s]!\n",
                            rc.toLatin1().constData(), inputQFile.fileName().toLatin1().constData());
#endif
                created->append(rc);
            }
        }
    }
    return true;
}

int
showHelp(const char *argv0, const QString &error)
{
    fprintf(stderr, "Qt resource compiler\n");
    if (!error.isEmpty())
        fprintf(stderr, "%s: %s\n", argv0, error.toLatin1().constData());
    fprintf(stderr, "Usage: %s  [options] <inputs>\n\n"
            "Options:\n"
            "\t-o file           Write output to file rather than stdout\n"
            "\t-name name        Create an external initialization function with name\n"
            "\t-threshold level  Threshold to consider compressing files\n"
            "\t-compress level   Compress input files by level\n"
            "\t-root path        Prefix resource acesspath with root path\n"
            "\t-no-compress      Disable all compression\n"
            "\t-version          Display version\n"
            "\t-help             Display this information\n",
            argv0);
    return 1;
}

int
main(int argc, char **argv)
{
    QString init_name, out_filename;
    bool show_help = false;
    QStringList files, display;

    //parse options
    QString error_msg;
    for (int i = 1; i < argc && error_msg.isEmpty(); i++) {
	if (argv[i][0] == '-') {   // option
            QByteArray opt = argv[i] + 1;
	    if (opt == "o") {
                if (!(i < argc-1)) {
                    error_msg = QLatin1String("Missing output name");
                    break;
                }
                out_filename = argv[++i];
            } else if(opt == "list") {
                if (!(i < argc-1)) {
                    error_msg = QLatin1String("Missing list description");
                    break;
                }
                display = QString(argv[++i]).simplified().split(',');
            } else if(opt == "name") {
                if (!(i < argc-1)) {
                    error_msg = QLatin1String("Missing target name");
                    break;
                }
                init_name = argv[++i];
            } else if(opt == "root") {
                if (!(i < argc-1)) {
                    error_msg = QLatin1String("Missing root path");
                    break;
                }
                resource_root = QDir::cleanPath(argv[++i]);
                if(resource_root.isEmpty() || resource_root[0] != '/')
                    error_msg = QLatin1String("Root must start with a /");
            } else if(opt == "compress") {
                if (!(i < argc-1)) {
                    error_msg = QLatin1String("Missing compression level");
                    break;
                }
                compress_level = QString(argv[++i]).toInt();
            } else if(opt == "threshold") {
                if (!(i < argc-1)) {
                    error_msg = QLatin1String("Missing compression threshold");
                    break;
                }
                compress_threshold = QString(argv[++i]).toInt();
            } else if(opt == "verbose") {
                verbose = true;
            } else if(opt == "version") {
		fprintf(stderr, "Resource Compiler for Qt version %s\n", QT_VERSION_STR);
		return 1;
            } else if(opt == "help" || opt == "h") {
                show_help = true;
            } else if(opt == "no-compress") {
                compress_level = 0;
            } else {
                error_msg = QString(QLatin1String("Unknown option: '%1'")).arg(argv[i]);
            }
        } else {
            if(!QFile::exists(argv[i])) {
                qWarning("%s: File does not exist '%s'", argv[0], argv[i]);
                return 1;
            }
            files.append(argv[i]);
        }
    }
    if (!files.size() || !error_msg.isEmpty() || show_help)
        return showHelp(argv[0], error_msg);

    //open output
    FILE *out_fd = stdout;
    if (!out_filename.isEmpty() && out_filename != "-") {
        out_fd = fopen(out_filename.toLocal8Bit().constData(), "w");
        if(!out_fd) {
            qWarning("%s: Could not open output file '%s'", argv[0], out_filename.toLocal8Bit().data());
            return 1;
        }
    }

    //render the display
    if(!display.isEmpty()) {
        for(int file = 0; file < files.count(); file++) {
            QList<RCCResource> resources = listResourceFile(files[file]);
            for(int resource = 0; resource < resources.size(); ++resource) {
                const RCCResource &r = resources.at(resource);
                if(display.indexOf("files") != -1 || display.indexOf("names") != -1) {
                    for(int f = 0; f < r.files.size(); ++f) {
                        for(int d = 0; d < display.size(); ++d) {
                            if(d)
                                fprintf(out_fd, ",");
                            if(display.at(d) == "files")
                                fprintf(out_fd, "%s", r.files.at(f).fileinfo.filePath().toLatin1().constData());
                            else if(display.at(d) == "names")
                                fprintf(out_fd, ":%s",
                                        QDir::cleanPath(resource_root + "/" +
                                                        r.prefix + "/" +
                                                        r.files.at(f).name).toLatin1().constData());
                            else if(display.at(d) == "langs")
                                fprintf(out_fd, "%s", r.lang.name().toLatin1().constData());
                        }
                        fprintf(out_fd, "\n");
                    }
                } else {
                    for(int d = 0; d < display.size(); ++d) {
                        if(d)
                            fprintf(out_fd, ",");
                        if(display.at(d) == "langs")
                            fprintf(out_fd, "%s", r.lang.name().toLatin1().constData());
                    }
                    fprintf(out_fd, "\n");
                }
            }
        }

        //close
        fclose(out_fd);
        return 0;
    }

    bool write_error = false;
    fprintf(out_fd, "/****************************************************************************\n");
    fprintf(out_fd, "** Resource object code\n");
    fprintf(out_fd, "**\n");
    fprintf(out_fd, "** Created: %s\n", QDateTime::currentDateTime().toString().toLatin1().data());
    fprintf(out_fd, "**      by: The Resource Compiler for Qt version %s\n", QT_VERSION_STR);
    fprintf(out_fd, "**\n");
    fprintf(out_fd, "** WARNING! All changes made in this file will be lost!\n");
    fprintf(out_fd, "*****************************************************************************/\n\n");
    fprintf(out_fd, "#include <qresource.h>\n");

    //process files
    QStringList all_resources;
    for(int file = 0; file < files.count(); file++) {
        if(!processResourceFile(files[file], out_fd, &all_resources)) {
            write_error = true;
            break;
        }
    }
    if(!write_error) {
        //initialization functions
        fprintf(out_fd, "//resource initialization function\n");
        bool no_name = init_name.isEmpty();
        if(no_name) { //need to make up one..
            init_name = out_filename;
            if(QDir::isRelativePath(init_name))
                init_name.prepend(QDir::currentPath() + "_");
        }
        init_name.replace(QRegExp("[^a-zA-Z0-9_]"), "_");
        if(no_name)
            fprintf(out_fd, "static ");
        fprintf(out_fd, "int qInitResources_%s()\n{\n", init_name.toLatin1().constData());
        for(int resource = 0; resource < all_resources.count(); resource++)
            fprintf(out_fd, "\t(void)%s();\n", all_resources[resource].toLatin1().constData());
        fprintf(out_fd, "\treturn %d;\n}\n", all_resources.count());
        fprintf(out_fd, "static int %s_static_init = qInitResources_%s();\n",
                init_name.toLatin1().constData(), init_name.toLatin1().constData());
    }

    //close
    fclose(out_fd);

    //done
    return write_error ? 11 : 0;
}
