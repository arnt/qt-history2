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

bool
processResourceFile(const QString &resource, QTextStream &out, QStringList *created)
{

    //read the resource
    QFile in(resource);
    if(!in.open(IO_ReadOnly)) {
        fprintf(stderr, "Unable to open %s", resource.latin1());
        return false;
    }
    QDomDocument document;
    document.setContent(&in);
    QDomElement root = document.firstChild().toElement();
    if(root.tagName() != QLatin1String("RCC")) {
        return false;
    }
    for(QDomElement child = root.firstChild().toElement(); !child.isNull(); 
        child = child.nextSibling().toElement()) {
        if(child.tagName() == QLatin1String("qresource")) {
            QFileInfoList files;
            QLocale lang(QLocale::C);
            QString prefix = child.attribute("prefix");
            if(child.hasAttribute("lang"))
                lang = QLocale(child.attribute("lang"));
            for(QDomNode res = child.firstChild(); !res.isNull(); res = res.nextSibling()) {
                if(res.toElement().tagName() == QLatin1String("file")) {
                    QFileInfo file(res.firstChild().toText().data());
                    if(!file.exists() || file.isDir()) {
                        QDir dir;
                        if(!file.exists()) 
                            dir = QDir(file.path(), file.fileName());
                        else 
                            dir = QDir(file.filePath(), "*");
                        QFileInfoList subFiles = dir.entryInfoList();
                        for(int subFile = 0; subFile < subFiles.count(); subFile++) {
                            if(subFiles[subFile].fileName() == "." || subFiles[subFile].fileName() == "..")
                                continue;
                            files.append(subFiles[subFile]);
                        }
                    } else {
                        files.append(file);
                    }
                }
            }
            for(int file = 0; file < files.count(); file++) {
                if(files[file].isDir()) { //do we want to get recursive?
                    continue;
                }

                //process this resource
                QFile inputQFile(files[file].filePath());
                if (!inputQFile.open(IO_ReadOnly)) {
                    qWarning("Could not open file '%s'", inputQFile.fileName().latin1());
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
                if(verbose)
                    qDebug("Read file %s [Compressed %d%%]", inputQFile.fileName().latin1(), 
                           compressRatio);

                //header
                const QString location = QDir::cleanPath(resource_root + "/" + 
                                                         prefix + "/" + 
                                                         inputQFile.fileName());
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
                    if(lang.language() != QLocale::C) {
                        resource_name += "__";
                        resource_name += lang.name();
                    }
                }
                uchar flags = 0;
                if(compressRatio)
                    flags |= Compressed;
                out << endl;
                out << "//Generated from '" << inputQFile.fileName().latin1() << "'" << endl;
                out << "static uchar " << resource_name << "[] = {" << endl;
                out << "\t0x12, 0x15, 0x19, 0x78, //header" << endl;
                out << "\t0x01, //version" << endl;
                out << "\t" << (uchar) lang.language() << ", " 
                    << (uchar)lang.country() << ", //lang" << endl;
                out << "\t" << flags << ", //flags" << endl;

                //name
                out << endl;
                out << "\t//name";
                for(int i = 0; i < location.length(); i++) {
                    if(!(i % 10))
                        out << "\n\t";
                    QChar c = location[i];
                    if(c == QDir::separator())
                        c = '/';
                    out << c.row() << ", " << c.cell() << ", ";
                }
                out << "\n\t0x00, 0x00, " << endl;

                //bits
                out << "\n\t//bits" << endl;
                uchar bytesNeeded = 0;
                const int input_length = input.length();
                for(int length = input_length; length > 0; length >>= 8)
                    bytesNeeded++;
                out << "\t" << bytesNeeded << ", //bytes in len\n\t";
                for(int i = bytesNeeded; i; i--)
                    out << ((input_length >> ((i-1)*8)) & 0xFF) << ", ";
                out << "//length";
                for(int i = 0; i < input_length; i++) {
                    if(!(i % 10))
                        out << "\n\t";
                    out << (uchar)input[i];
                    if(i != input_length-1)
                        out << ", ";
                }

                //footer
                out << "\n};" << endl;

                //QMetaResource
                out << "Q_GLOBAL_STATIC_WITH_ARGS(QMetaResource, resource_" 
                    << resource_name << ", (" << resource_name << "))" << endl;
                if(created)
                    created->append("resource_" + resource_name);
            }
        }
    }
    return true;
}

int
main(int argc, char **argv)
{
    QString init_name, output_file;
    bool show_help = false;
    QStringList files;

    //parse options
    char *error_msg = 0;
    for (int i = 1; i < argc && !error_msg; i++) {
	if (argv[i][0] == '-') {   // option
            QByteArray opt = argv[i] + 1;
	    if (opt == "o") {
                if (!(i < argc-1)) {
                    error_msg = "Missing output name";
                    break;
                }
                output_file = argv[++i];
            } else if(opt == "name") {
                if (!(i < argc-1)) {
                    error_msg = "Missing target name";
                    break;
                }
                init_name = argv[++i];
            } else if(opt == "root") {
                if (!(i < argc-1)) {
                    error_msg = "Missing root path";
                    break;
                }
                resource_root = QDir::cleanPath(argv[++i]);
                if(resource_root.isEmpty() || resource_root[0] != '/')
                    error_msg = "Root must start with a /";
            } else if(opt == "compress") {
                if (!(i < argc-1)) {
                    error_msg = "Missing compression level";
                    break;
                }
                compress_level = QString(argv[++i]).toInt();
            } else if(opt == "threshold") {
                if (!(i < argc-1)) {
                    error_msg = "Missing compression threshold";
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
                error_msg = "Unknown option";
            }
        } else {
            if(!QFile::exists(argv[i])) {
                qWarning("%s: File does not exist '%s'", argv[0], argv[i]);
                return 1;
            }
            files.append(argv[i]);
        }
    }
    if (!files.size() || error_msg || show_help) {
	fprintf(stderr, "Qt resource compiler\n");
	if (error_msg)
	    fprintf(stderr, "%s: %s\n", argv[0], error_msg);
        fprintf(stderr, "Usage: %s  [options] <inputs>\n\n"
                "Options:\n"
                "\t-o file           Write output to file rather than stdout\n"
                "\t-name name        Create an external initialization function with name\n"
                "\t-target targ      Create initialization function for targ\n"
                "\t-threshold level  Threshold to consider compressing files\n"
                "\t-compress level   Compress input files by level\n"
                "\t-root path        Prefix resource acesspath with root path\n"
                "\t-no-compress      Disable all compression\n"
                "\t-r                Perform compilation recursivley on directories\n"
                "\t-version          Display version\n"
                "\t-help             Display this information\n",
                argv[0]);
        return 1;
    }

    //open output
    bool asTempFile = false;
    QIODevice *out_dev = 0;
    if (!output_file.isEmpty() && output_file != "-") {
        out_dev = new QTemporaryFile;
        if(!out_dev->open(QFile::ReadWrite)) {
            delete out_dev;
            out_dev = new QFile(output_file.utf8());
            if(!out_dev->open(QIODevice::WriteOnly)) {
                qWarning("%s: Could not open output file '%s'", argv[0], output_file.latin1());
                return 1;
            }
        } else {
            asTempFile = true;
        }
    } else {
        QFile *file = new QFile;
        out_dev = file;
        if(!file->open(QFile::WriteOnly, stdout)) {
            delete file;
            return 1;
        }
    }
    bool write_error = false;
    QTextStream out(out_dev);
    out.setf(QTextStream::showbase|QTextStream::hex);
    out <<  "/****************************************************************************" << endl;
    out << "** Resource object code" << endl;
    out << "**" << endl;
    out << "** Created: " << QDateTime::currentDateTime().toString().latin1() << endl;
    out << "**      by: The Resource Compiler for Qt version " << QT_VERSION_STR << endl;
    out << "**" << endl;
    out << "** WARNING! All changes made in this file will be lost!" << endl;
    out << "*****************************************************************************/" << endl << endl;
    out << "#include <qresource.h>" << endl;

    //process files
    QStringList all_resources;
    for(int file = 0; file < files.count(); file++) {
        if(!processResourceFile(files[file], out, &all_resources)) {
            write_error = true;
            break;
        }
    }
    if(!write_error) {
        //initialization functions
        out << "//resource initialization function" << endl;
        bool no_name = init_name.isEmpty();
        if(no_name) { //need to make up one..
            init_name = output_file;
            if(QDir::isRelativePath(init_name))
                init_name.prepend(QDir::currentPath() + "_");
        }
        init_name.replace(QRegExp("[^a-zA-Z0-9_]"), "_");
        if(no_name)
            out << "static ";
        out << "int qInitResources_" << init_name << "()" << endl << "{" << endl;
        for(int resource = 0; resource < all_resources.count(); resource++)
            out << "\t(void)" << all_resources[resource] << "();" << endl;
        out << "\treturn " << all_resources.count() << ";" << endl << "}" << endl;
        out << "static int " << init_name << "_static_init = qInitResources_" << init_name << "();" << endl;
    }

    //close
    out_dev->close();
    if(!write_error && asTempFile) {
        QTemporaryFile *temp = static_cast<QTemporaryFile*>(out_dev);
        temp->rename(output_file);
    }
    delete out_dev;
    out_dev = 0;

    //done
    return 0;
}
