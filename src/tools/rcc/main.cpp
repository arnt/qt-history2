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

// Any changes here must be reflected in qresource.cpp
enum {
    Compressed = 0x01
};

int
main(int argc, char **argv)
{
    bool show_help = false, recursive = false,
	   verbose = false, path = true, force_relative = true;
    char *error = 0;
    int compress_level = -1, compress_threshold = 70;
    QString output_file, prefix, init_name;
    QFileInfoList files;

    QString currentPath = QDir::currentPath();

    //parse options
    for (int i = 1; i < argc && !error; i++) {
	if (argv[i][0] == '-') {   // option
            QByteArray opt = argv[i] + 1;
	    if (opt == "o") {
                if (!(i < argc-1)) {
                    error = "Missing output name";
                    break;
                }
                output_file = argv[++i];
            } else if(opt == "name") {
                if (!(i < argc-1)) {
                    error = "Missing target name";
                    break;
                }
                init_name = argv[++i];
            } else if(opt == "prefix") {
                if (!(i < argc-1)) {
                    error = "Missing prefix path";
                    break;
                }
                prefix = QDir::cleanPath(argv[++i]);
                if(prefix.isEmpty() || prefix[0] != '/')
                    error = "Prefix must start with a /";
            } else if(opt == "compress") {
                if (!(i < argc-1)) {
                    error = "Missing compression level";
                    break;
                }
                compress_level = QString(argv[++i]).toInt();
            } else if(opt == "threshold") {
                if (!(i < argc-1)) {
                    error = "Missing compression threshold";
                    break;
                }
                compress_threshold = QString(argv[++i]).toInt();
            } else if(opt == "r") {
                recursive = true;
            } else if(opt == "verbose") {
                verbose = true;
            } else if(opt == "version") {
		fprintf(stderr, "Resource Compiler for Qt version %s\n", QT_VERSION_STR);
		return 1;
            } else if(opt == "help" || opt == "h") {
                show_help = true;
            } else if(opt == "no-compress") {
                compress_level = 0;
            } else if(opt == "no-path") {
                path = false;
            } else if(opt == "no-relative-path") {
                force_relative = false;
            } else {
                error = "Unknown option";
            }
        } else {
            QFileInfo fi(argv[i]);
            if(!fi.exists()) {
                qWarning("%s: File does not exist '%s'", argv[0], argv[i]);
                return 1;
            }

            if (force_relative) {
                QString stripped = fi.absoluteFilePath();
                if (stripped.startsWith(currentPath)) {
                    stripped = stripped.mid(currentPath.length());
                    while (stripped.length() && stripped.at(0) == '/') {
                        stripped = stripped.mid(1);
                    }
                }

                if (stripped.isEmpty())
                    stripped = ".";
                files.append(QFileInfo(stripped));
            } else {
                files.append(fi);
            }
        }
    }
    if (!files.size() || error || show_help) {
	fprintf(stderr, "Qt resource compiler.\n");
	if (error)
	    fprintf(stderr, "%s: %s\n", argv[0], error);
        fprintf(stderr, "Usage: %s  [options] <inputs>\n\n"
                "Options:\n"
                "\t-o file           Write output to file rather than stdout\n"
                "\t-target targ      Create initialization function for targ\n"
                "\t-threshold level  Threshold to consider compressing files\n"
                "\t-compress level   Compress input files by level\n"
                "\t-prefix path      Prefix resource acesspath with path\n"
                "\t-no-compress      Disable all compression\n"
                "\t-r                Perform compilation recursivley on directories\n"
                "\t-version          Display version\n"
                "\t-help             Display this information\n",
                argv[0]);
        return 1;
    }

    //open output
    FILE *out = stdout;
    if (!output_file.isEmpty() && output_file != "-") {
	if (!(out = fopen(output_file.utf8(), "w"))) {
	    qWarning("%s: Could not open output file '%s'", argv[0], output_file.latin1());
	    return 1;
	}
    }
    fprintf(out, "/****************************************************************************\n");
    fprintf(out, "** Resource object code\n");
    fprintf(out, "**\n");
    fprintf(out, "** Created: %s\n", QDateTime::currentDateTime().toString().latin1());
    fprintf(out, "**      by: The Qt Resource Compiler ($Id: $)\n");
    fprintf(out, "**\n");
    fprintf(out, "** WARNING! All changes made in this file will be lost!\n");
    fprintf(out, "*****************************************************************************/\n");
    fprintf(out, "\n#include <qresource.h>\n");

    //process files
    QStringList global_resources;
    for(int file = 0; file < files.size(); file++) {
        if(files[file].isDir()) {
            QDir dir(files[file].filePath());
            QFileInfoList subFiles = dir.entryInfoList(QStringList("*"));
            for(int subFile = 0; subFile < subFiles.count(); subFile++) {
                if(subFiles[subFile].fileName() == "." || subFiles[subFile].fileName() == "..")
                    continue;
                if(recursive || !subFiles[subFile].isDir())
                    files.append(subFiles[subFile]);
            }
        } else {
            QFile inputQFile(files[file].filePath());
            if (!inputQFile.open(IO_ReadOnly)) {
                qWarning("%s: Could not open file '%s'", argv[0], files[file].filePath().latin1());
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
                qDebug("Read file %s [Compressed %d%%]", files[file].filePath().latin1(), compressRatio);

            //header
            const QString resource = QDir::cleanPath(prefix + "/" + (path ? files[file].filePath() : files[file].fileName()));
            QString resource_name = resource;
            resource_name.replace(QRegExp("[^a-zA-Z0-9_]"), "_");
            uchar flags = 0;
            if(compressRatio)
                flags |= Compressed;
            fprintf(out, "\n//Generated from '%s'\n", files[file].filePath().latin1());
            fprintf(out, "static uchar %s[] = {\n", resource_name.latin1());
            fprintf(out, "\t0x12, 0x15, 0x19, 0x78, //header\n");
            fprintf(out, "\t0x01, //version\n");
            fprintf(out, "\t0x%02x, //flags\n", flags);

            //name
            fprintf(out, "\n\t//name\n\t");;
            for(int i = 0; i < resource.length(); i++) {
                if(i && !(i % 10))
                    fprintf(out, "\n\t");
		QChar c = resource[i];
		if(c == QDir::separator())
		    c = '/';
                fprintf(out, "0x%02x, 0x%02x, ", c.row(), c.cell());
            }
            fprintf(out, "\n\t0x00, 0x00, \n");

            //bits
            fprintf(out, "\n\t//bits\n");
            uchar bytesNeeded = 0;
            const int input_length = input.length();
            for(int length = input_length; length > 0; length >>= 8)
                bytesNeeded++;
            fprintf(out, "\t0x%02x, //bytes in len\n\t", bytesNeeded);
            for(int i = bytesNeeded; i; i--)
                fprintf(out, "0x%02x, ", (input_length >> ((i-1)*8)) & 0xFF);
            fprintf(out, "//length\n\t");
            for(int i = 0; i < input_length; i++) {
                if(i && !(i % 10))
                    fprintf(out, "\n\t");
                fprintf(out, "0x%02x", (uchar)input[i]);
                if(i != input_length-1)
                    fprintf(out, ", ");
            }

            //footer
            fprintf(out, "\n};\n");

            //QMetaResource
            fprintf(out, "Q_GLOBAL_STATIC_WITH_ARGS(QMetaResource, resource_%s, (%s))\n",
                    resource_name.latin1(), resource_name.latin1());
            global_resources << "resource_" + resource_name;
        }
    }
    //initialization functions
    bool no_name = init_name.isEmpty();
    if(no_name) { //need to make up one..
        init_name = output_file;
        if(QDir::isRelativePath(init_name))
            init_name.prepend(QDir::currentPath() + "_");
    }
    init_name.replace(QRegExp("[^a-zA-Z0-9_]"), "_");
    fprintf(out, "\n//resource initialization function\n");
    fprintf(out, "%sint qInitResources_%s()\n{\n",
            no_name ? "static " : "", init_name.latin1());
    for(int resource = 0; resource < global_resources.count(); resource++)
        fprintf(out, "\t(void)%s();\n", global_resources[resource].latin1());
    fprintf(out, "\treturn %d;\n}\n", global_resources.count());
    fprintf(out, "static int %s_static_init = qInitResources_%s();\n",
            init_name.latin1(), init_name.latin1());
    //close
    fclose(out);

    //done
    return 0;
}

