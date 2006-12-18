/************************************************************
 * Some global variables needed throughout the packaging script.
 */
const qtDir = System.getenv("QTDIR");
const qmakeCommand = qtDir + "/bin/qmake";

const qdocDir = qtDir + "/util/qdoc3";
const qdocCommand = qdocDir + "/qdoc3";

const outputDir = System.getenv("PWD");

const validPlatforms = ["win", "x11", "mac", "core", "all"];
const validLicenses = ["opensource", "commercial", "eval"];
const validSwitches = ["gzip", "bzip", "zip", "snapshots", "preview"]; // these are either true or false, set by -do-foo/-no-foo
const validVars = ["branch", "version", "label"]; // variables with arbitrary values, set by -foo value

const binaryExtensions = ["msi", "dll", "gif", "png", "mng",
                          "jpg", "bmp", "any", "pic", "ppm",
                          "exe", "zip", "qm", "ico", "wav",
                          "icns", "qpf", "bdf", "pfb", "pfa",
                          "ttf", "resource"];

const user = System.getenv("USER");

var startDate = new Date(); // the start date of the script
var options = [];            // list of all package options
var tmpDir;                    // directory for system temporary files
var distDir;                    // parent directory for all temp packages and the checkout dir
var checkoutDir;            // directory for P4 checkout
var licenseHeaders = [];    // license text to put in .cpp and .h headers
var moduleMap = [];         // maps between directories and module/class/application names
var p4Port;
var p4Command;
var p4BranchPath;            // typically //depot/qt/[thebranch]
var p4Label;                 // the P4 label or date

var indentation = 0;
const tabSize = 4;

/************************************************************
 * Purging filters
 */

var checkoutRemove = [ new RegExp("^LICENSE.TROLL"),
                       new RegExp("^bin/syncqt.bat"),
                       new RegExp("^mkspecs/features/qttest_p4.prf"),
                       new RegExp("^pics"),
                       new RegExp("^src/corelib/eval.pri"),
                       new RegExp("^src/corelib/kernel/qtcore_eval.cpp"),
                       new RegExp("^src/gui/painting/makepsheader.pl"),
                       new RegExp("^src/gui/painting/qpsprinter"),
                       new RegExp("^src/gui/styles/qmotifplus"),
                       new RegExp("^src/gui/styles/qplatinum"),
                       new RegExp("^src/gui/styles/qsgi"),
                       new RegExp("^src/plugins/styles/motifplus"),
                       new RegExp("^src/plugins/styles/platinum"),
                       new RegExp("^src/plugins/styles/sgi"),
                       new RegExp("^tests"),
                       new RegExp("^tmake"),
                       new RegExp("^tools/designer/data"),
                       new RegExp("^tools/designer/tests"),
                       new RegExp("^tools/linguist/.*\\.1"),
                       new RegExp("^tools/mergetr"),
                       new RegExp("^tools/msg2qm"),
                       new RegExp("^tools/qembed"),
                       new RegExp("^tools/qev"),
                       new RegExp("^util") ];

var platformRemove = new Array();

platformRemove["win"] = [ new RegExp("\\.qws"),
                          new RegExp("^LICENSE.QPL"),
                          new RegExp("^config.tests"),
                          new RegExp("^configure$"),
                          new RegExp("^doc/src"),
                          new RegExp("^examples/qtopiacore"),
                          new RegExp("^gif"),
                          new RegExp("^qmake/Makefile.commercial"),
                          new RegExp("^qmake/Makefile.unix"),
                          new RegExp("^src/plugins/decorations"),
                          new RegExp("^src/plugins/gfxdrivers"),
                          new RegExp("^src/plugins/mousedrivers"),
                          new RegExp("^src/plugins/styles/mac"),
                          new RegExp("^src/gui/embedded"),
                          new RegExp("^tools/qconfig"),
                          new RegExp("^tools/qtconfig"),
                          new RegExp("^tools/qvfb"),
                          new RegExp("_mac"),
                          new RegExp("_qnx4"),
                          new RegExp("_qnx6"),
                          new RegExp("_qws"),
                          new RegExp("_unix"),
                          new RegExp("_wce"),
                          new RegExp("_x11") ];

platformRemove["x11"] = [ new RegExp("\\.qws"),
                          new RegExp("^configure.exe"),
                          new RegExp("^doc/src"),
                          new RegExp("^examples/activeqt"),
                          new RegExp("^examples/qtopiacore"),
                          new RegExp("^gif"),
                          new RegExp("^qmake/Makefile$"),
                          new RegExp("^qmake/Makefile.win32-g++"),
                          new RegExp("^src/activeqt"),
                          new RegExp("^src/plugins/decorations"),
                          new RegExp("^src/plugins/gfxdrivers"),
                          new RegExp("^src/plugins/mousedrivers"),
                          new RegExp("^src/plugins/styles/mac"),
                          new RegExp("^src/tools/idc"),
                          new RegExp("^src/winmain"),
                          new RegExp("^src/gui/embedded"),
                          new RegExp("^tools/activeqt"),
                          new RegExp("^tools/configure"),
                          new RegExp("^tools/designer/src/plugins/activeqt"),
                          new RegExp("_mac"),
                          new RegExp("_qnx4"),
                          new RegExp("_qnx6"),
                          new RegExp("_qws"),
                          new RegExp("_wce"),
                          new RegExp("_win") ];

platformRemove["mac"] = [ new RegExp("\\.qws"),
                          new RegExp("^LICENSE.QPL"),
                          new RegExp("^configure.exe"),
                          new RegExp("^doc/src"),
                          new RegExp("^examples/activeqt"),
                          new RegExp("^examples/qtopiacore"),
                          new RegExp("^gif"),
                          new RegExp("^qmake/Makefile$"),
                          new RegExp("^qmake/Makefile.win32-g++"),
                          new RegExp("^src/activeqt"),
                          new RegExp("^src/plugins/decorations"),
                          new RegExp("^src/plugins/gfxdrivers"),
                          new RegExp("^src/plugins/mousedrivers"),
                          new RegExp("^src/tools/idc"),
                          new RegExp("^src/winmain"),
                          new RegExp("^src/gui/embedded"),
                          new RegExp("^tools/activeqt"),
                          new RegExp("^tools/configure"),
                          new RegExp("^tools/designer/src/plugins/activeqt"),
                          new RegExp("^tools/qconfig"),
                          new RegExp("^tools/qtconfig"),
                          new RegExp("^tools/qvfb"),
                          new RegExp("_qnx4"),
                          new RegExp("_qnx6"),
                          new RegExp("_qws"),
                          new RegExp("_wce"),
                          new RegExp("_win"),
                          new RegExp("_x11") ];

platformRemove["core"] = [ new RegExp("^LICENSE.QPL"),
                           new RegExp("^configure.exe"),
                           new RegExp("^doc/src"),
                           new RegExp("^examples/activeqt"),
                           new RegExp("^gif"),
                           new RegExp("^qmake/Makefile$"),
                           new RegExp("^qmake/Makefile.win32-g++"),
                           new RegExp("^src/activeqt"),
                           new RegExp("^src/plugins/styles/mac"),
                           new RegExp("^src/tools/idc"),
                           new RegExp("^src/winmain"),
                           new RegExp("^tools/activeqt"),
                           new RegExp("^tools/configure"),
                           new RegExp("^tools/designer/src/plugins/activeqt"),
                           new RegExp("^tools/qvfb"),
                           new RegExp("_mac"),
                           new RegExp("_qnx4"),
                           new RegExp("_qnx6"),
                           new RegExp("_wce"),
                           new RegExp("_win") ];

platformRemove["all"] = [  new RegExp("^doc/src"),
                           new RegExp("_qnx4"),
                           new RegExp("_qnx6"),
                           new RegExp("_wce") ];

var licenseRemove = new Array();

licenseRemove["commercial"] = [ new RegExp("LICENSE.GPL") ];

licenseRemove["eval"] = [ new RegExp("LICENSE.GPL"),
                          new RegExp("^bin/qtusagereporter") ];

licenseRemove["opensource"] = [ new RegExp("^README-QT.TXT"),
                                new RegExp("^README.qws"),
                                new RegExp("^bin/qtusagereporter"),
                                new RegExp("^examples/activeqt"),
                                new RegExp("^mkspecs/macx-mwerks"),
                                new RegExp("^mkspecs/win32-borland"),
                                new RegExp("^mkspecs/win32-icc"),
                                new RegExp("^mkspecs/win32-msvc"),
                                new RegExp("^mkspecs/win32-msvc.net"),
                                new RegExp("^qmake/Makefile$"),
                                new RegExp("^qmake/Makefile.commercial"),
                                new RegExp("^qmake/generators/mac/metrowerks"),
                                new RegExp("^qmake/generators/win32/borland"),
                                new RegExp("^qmake/generators/win32/msvc"),
                                new RegExp("^src/plugins/sqldrivers/db2"),
                                new RegExp("^src/plugins/sqldrivers/oci"),
                                new RegExp("^src/plugins/sqldrivers/tds"),
                                new RegExp("^src/sql/drivers/db2"),
                                new RegExp("^src/sql/drivers/oci"),
                                new RegExp("^src/sql/drivers/tds"),
                                new RegExp("^src/tools/idc"),
                                new RegExp("^tools/activeqt"),
                                new RegExp("^tools/designer/src/plugins/activeqt"),
                                new RegExp("^src/activeqt") ];

licenseRemove["preview-opensource"] = licenseRemove["opensource"];
licenseRemove["preview-commercial"] = licenseRemove["commercial"];

licenseRemove["snapshot-opensource"] = licenseRemove["opensource"];
licenseRemove["snapshot-commercial"] = licenseRemove["commercial"];

var binaryFileList = [ new RegExp("examples/tools/codecs/encodedfiles/utf-16.txt"),
                       new RegExp("examples/tools/codecs/encodedfiles/utf-16be.txt"),
                       new RegExp("examples/tools/codecs/encodedfiles/utf-16le.txt"),
                       new RegExp("tools/designer/src/designer/extra/names.txt") ];

var finalRemove = [ new RegExp("^dist"),
                    new RegExp("^doc/html/qt.index") ];

/************************************************************
 * Mapping from directories to module names
 */

// main modules
moduleMap["Qt3Support module"]           = new RegExp("^src/qt3support");
moduleMap["QtCore module"]               = new RegExp("^src/core");
moduleMap["QtGui module"]                 = new RegExp("^src/gui");
moduleMap["QtNetwork module"]            = new RegExp("^src/network");
moduleMap["QtOpenGL module"]             = new RegExp("^src/opengl");
moduleMap["QtSql module"]                = new RegExp("^src/sql");
moduleMap["QtXML module"]                = new RegExp("^src/xml");
moduleMap["QtSVG module"]                = new RegExp("^src/svg");
moduleMap["ActiveQt Framework"]          = new RegExp("^src/activeqt");

// main applications
moduleMap["Qt Designer"]                 = new RegExp("^tools/designer");
moduleMap["Qt Assistant"]                = new RegExp("^tools/assistant");
moduleMap["Qt Linguist"]                 = new RegExp("^tools/linguist");
moduleMap["qt3to4 porting application"]  = new RegExp("^tools/porting");
moduleMap["qmake application"]           = new RegExp("^qmake");

// other
moduleMap["demonstration applications"]  = new RegExp("^demos");
moduleMap["documentation"]               = new RegExp("^doc");
moduleMap["example classes"]             = new RegExp("^examples");
moduleMap["tools applications"]          = new RegExp("(^src/tools|^tools/)");
moduleMap["window classes"]              = new RegExp("^src/winmain");
moduleMap["plugins"]                     = new RegExp("^src/plugins");
moduleMap["qmake spec"]                  = new RegExp("^mkspec");

/*******************************************************************************
 * Here we go
 */
print("Initializing...");
parseArgc();
initialize();
print("Checking tools and hosts...");
checkTools();
print("Building qdoc...");
buildQdoc();
print("Checkout from P4...");
preparePerforce();
checkoutDir = checkout("...", distDir + "/qt");
print("Purging checkout...");
purgeFiles(checkoutDir, getFileList(checkoutDir), checkoutRemove);
indentation+=tabSize;
for (var p in validPlatforms) {
    for (var l in validLicenses) {
        var platform = validPlatforms[p];
        var license = validLicenses[l];
        if (options[platform] && options[license] &&
            (license != "eval" || (platform == "x11" || platform == "core"))) {
            print("Packaging %1-%2...".arg(platform).arg(license));
            indentation+=tabSize;
            
            // copy checkoutDir to platDir and set permissions
            print("Copying checkout...");
            var platName = "qt-%1-%2-src-%3"
                .arg(platform)
                .arg(license)
                .arg(options["version"]);
            if (platform == "core")
                platName = platName.replace("qt-core", "qtopia-core");
            var platDir = distDir + "/" + platName;
            execute(["cp", "-r", checkoutDir, platDir]);
            
            //copying dist files
            print("Copying dist files...");
            copyDist(platDir, platform, license);
            copyTarget(platDir, platform);

            checkLicense(platDir, getFileList(platDir), "first");

            // replace tags (like THISYEAR etc.)
            print("Traversing all txt files and replacing tags...");
            replaceTags(platDir, getFileList(platDir), defaultTags(platform, license, platName));

            // run qdoc
            print("Running qdoc...");
            qdoc(platDir, platform, license);
            replaceTags(platDir + "/doc", getFileList(platDir + "/doc"),
                        defaultTags(platform, license, platName));

            // purge platform and license files
            print("Purging platform and license specific files...");
            purgeFiles(platDir, getFileList(platDir),[]
                       .concat(platformRemove[platform])
                       .concat(licenseRemove[license]));

            checkLicense(platDir, getFileList(platDir), "final");

            // run syncqt
            print("Running syncqt...");
            syncqt(platDir, platform);

            // final package purge
            print("Final package purge...");
            purgeFiles(platDir, getFileList(platDir), finalRemove);

            // package directory
            print("Compressing and packaging file(s)...");
            compress(platform, platDir, platName);

            // create eval patches for win and mac
            if (options["eval"] && license == "commercial" &&
                (platform == "win" || platform == "mac")) {
                // delete and make an empty platDir
                var dir = new Dir(platDir);
                if (dir.exists)
                    dir.rmdirs();
                dir.mkdir();
                copyEval(platDir);
                replaceTags(platDir, getEvalFileList(platDir), defaultTags(platform, license, platName));
                compress(platform, platDir, platName.replace("commercial", "evalpatches"));
            }
            indentation-=tabSize;
        }
    }
 }

indentation-=tabSize;
print("Cleaning all temp files...");
cleanup();

/************************************************************
 * Parses and checks the commandline options and puts them into options[key] = value
 */
function parseArgc()
{
    var validOptions = []
        .concat(validPlatforms)
        .concat(validLicenses)
        .concat(validSwitches)
        .concat(validVars);
    for (var i=0; i<argc.length; ++i) {
        var optionKey;
        var optionValue;
        if (argc[i].startsWith("-do")) {
            optionKey = argc[i].mid(4);
            optionValue = true;
        } else if (argc[i].startsWith("-no")) {
            optionKey = argc[i].split("-")[2];
            optionValue = false;
        } else if (argc[i].startsWith("-")) {
            optionKey = argc[i].split("-")[1];
            optionValue = argc[++i];
        } else {
            throw "Invalid option format: %1".arg(argc[i]);
        }

        var optionOk = false;
        for (var o in validOptions) {
            if (optionKey == validOptions[o]) {
                optionOk = true;
                break;
            }
        }

        if (optionOk)
            options[optionKey] = optionValue;
        else
            throw "Unknown option: %1".arg(optionKey);
    }
}

/************************************************************
 * Setup directories and query environment variables
 */
function initialize()
{
    if (!("label" in options)) {
        /* label is an optional argument */
        options["label"] = "";
    }

    // checks that all valid vars are specified
    for (var i in validVars)
        if ((!(validVars[i] in options)))
            throw "%1 was not specified.".arg(validVars[i]);

    // by default turn off all valid switches that were not defined
    for (var i in validSwitches)
        if (!(validSwitches[i] in options))
            options[validSwitches[i]] = false;

    // by default turn off all valid platforms that were not defined
    for (var i in validPlatforms)
        if (!(validPlatforms[i] in options))
            options[validPlatforms[i]] = false;

    // by default turn off all valid licenses that were not defined
    for (var i in validLicenses)
        if (!(validLicenses[i] in options))
            options[validLicenses[i]] = false;

    // make sure platform and license filters are defined
    for (var i in validPlatforms) {
        if (!(validPlatforms[i] in platformRemove))
            platformRemove[validPlatforms[i]] = new Array();
    }
    for (var i in validLicenses) {
        if (!(validLicenses[i] in licenseRemove))
            licenseRemove[validLicenses[i]] = new Array();
    }

    // finds a tmpDir
    if (tmpDir == undefined || !File.exists(tmpDir)) {
        if (File.exists("/tmp"))
            tmpDir = "/tmp";
        else if (File.exists(System.getenv("HOME") + "/tmp"))
            tmpDir = System.getenv("HOME") + "/tmp";
        else
            throw "Unable to find tmp directory";
    }
    // creates distDir and sets checkoutDir
    distDir = tmpDir + "/qt-" + options["branch"] + "-" + user + "-" + startDate.getTime();
    var dir = new Dir(distDir);
    if (dir.exists)
        dir.rmdirs();
    dir.mkdir();

    // setting up p4
    if (p4Port == undefined)
        p4Port = "p4.troll.no:866";
    if (p4Command == undefined || !File.exists(p4Command))
        p4Command = System.getenv("which p4");
    if (!File.exists(p4Command))
        p4Command = "/usr/local/bin/p4";
    if (!File.exists(p4Command))
        p4Command = "/usr/bin/p4";

    // add "-snapshot-yyyymmdd" to version
    if (options["snapshots"])
        options["version"] = options["version"] + "-snapshot-%1%2%3"
            .arg(startDate.getYear())
            .arg(startDate.getMonth() < 10 ? "0" + startDate.getMonth() : startDate.getMonth())
            .arg(startDate.getDate() < 10 ? "0" + startDate.getDate() : startDate.getDate());

//     for (var i in options)
//         print("options[%1] = %2".arg(i).arg(options[i]));
}

/************************************************************
 * Verify that the necessary tools and hosts are available.
 */
function checkTools()
{
    try {
        execute(["which", qmakeCommand]);
        execute("zip -help");
        execute("tar --help");
        execute("gzip -h");
         execute("bzip2 -h");
        execute("cp --help");
        execute("which scp");
        execute(p4Command);
    } catch (e) {
        throw "Tool failed: %1".arg(e);
    }
}

/************************************************************
 * Builds and checks qdoc
 */
function buildQdoc()
{
    var dir = new Dir(qdocDir);
    dir.setCurrent();
    execute("%1 qdoc3.pro".arg(qmakeCommand));
    execute("make");
    // test qdoc
    execute( [qdocCommand, "-version"] );
}


/************************************************************
 * checks that branch and version exists and sets the p4Label etc.
 */
function preparePerforce()
{
    // check that the branch exist
    p4BranchPath = "//depot/qt/" + options["branch"];
    execute([p4Command, "fstat", p4BranchPath + "/configure"]);
    if (Process.stdout.find("depotFile") == -1)
        throw "Branch: " + p4BranchPath + " does not exist.";
    
    // check that the label exists
    if (options["snapshots"]) {
        if (options["label"]) {
            p4Label = options["label"];
        } else {
            p4Label = startDate.toString().replace(/-/g, "/").replace(/T/g, ":");
        }
    } else {
        p4Label = "qt/" + options["version"];
        execute([p4Command, "labels", p4BranchPath + "/configure"]);
        if (Process.stdout.find("Label " + p4Label + " ") == -1)
            throw "Label: " + p4Label + " does not exist, or not in this branch.";
    }
}


/************************************************************
 * checks out p4Path (for example "...") from P4 and puts it into the absolute
 * directory localDir, returns the localDir on success or throws an exception
 *
 * Note: localDir is made writable
 */
function checkout(p4Path, localDir)
{
    // generate clientSpec
    var tmpClient="qt-release-tmp-" + user;
    execute([p4Command, "client", "-t", "qt-release-3x", "-o", tmpClient]);
    var clientSpec = Process.stdout.split("\n");
    for (var i in clientSpec) {
        clientSpec[i] = clientSpec[i].replace(/^Root:.*/, "Root: " + localDir);
        clientSpec[i] = clientSpec[i].replace(/X.Y.*/, options["branch"] + "/" + p4Path + " " +
                                              "//" + tmpClient + "/...");
        clientSpec[i] = clientSpec[i].replace(/\bnomodtime\b/, "modtime");
    }
    // save clientSpec
    clientSpec = clientSpec.join("\n");
    execute([p4Command, "client", "-i"], clientSpec);

    // checkout
    execute([p4Command, "-c", tmpClient, "-d", localDir, "sync", "-f", "...@" + p4Label]);

    // test that checkout worked
    if (!File.exists(localDir))
        throw "Checkout failed, checkout dir %1 does not exist.".arg(checkoutDir);
    execute(["chmod", "-R", "ug+w", localDir]);
    return localDir;
}

/************************************************************
 * iterates over the fileList and removes any files found in the
 * remove patterns
 */
function purgeFiles(rootDir, fileList, remove)
{
    var doRemove = false;
    var fileName = new String();
    var absFileName = new String();

    for (var i in fileList) {
        doRemove = false;
        fileName = fileList[i];
        absFileName = rootDir + "/" + fileName;
        // check if the file should be removed
        for (var r in remove) {
            if (fileName.find(remove[r]) != -1) {
                doRemove = true;
                break;
            }
        }

        // remove file
        if (doRemove && File.exists(absFileName)) {
            if (File.isFile(absFileName)) {
                File.remove(absFileName);
            } else if (File.isDir(absFileName)) {
                var dir = new Dir(absFileName);
                dir.rmdirs();
            }
        }
    }
}

/************************************************************
 * compresses platDir into files (.zip .gz etc.)
 */
function compress(platform, packageDir, packageName)
{
    // set directory to parent of packageDir
    var dir = new Dir(packageDir);
    var packageDirName = dir.name;
    dir.cdUp();
    dir.setCurrent();

    if (platform == "win") {
        if (options["zip"]) {
            var files = getFileList(packageDir);
            var binaryFiles = new Array();
            var textFiles = new Array();
            var fileName = new String();
            var absFileName = new String();
            var zipFile = outputDir + "/" + packageName + ".zip";
            // delete any old zipFile
            if (File.exists(zipFile))
                File.remove(zipFile);
            // generate list of binary and text files
            for (var i in files) {
                fileName = files[i];
                absFileName = packageDir + "/" + fileName;
                if (File.exists(absFileName) && File.isFile(absFileName)) {
                    if (isBinaryFile(absFileName))
                        binaryFiles.push(packageDirName + "/" + fileName);
                    else
                        textFiles.push(packageDirName + "/" + fileName);
                }
            }
            // add the binary and text files to the zip file in in two big goes
            dir.setCurrent(); //  current dir is parent of packageDir
            if (binaryFiles.length > 0)
                execute(["zip", "-9q", zipFile, "-@"], binaryFiles.join("\n"));
            if (textFiles.length > 0)
                execute(["zip", "-l9q", zipFile, "-@"], textFiles.join("\n"));
        }
    } else {
        var tarFile = outputDir + "/" + packageName + ".tar";
        execute(["tar", "-cf", tarFile, packageDirName]);
        if (!File.exists(tarFile))
            throw "Failed to produce %1.".arg(tarFile);
        
         if (options["bzip"]) {
             execute(["bzip2", "-zkf", tarFile]);
         }
         if (options["gzip"]) {
             execute(["gzip", "-f", tarFile]);
        }
        // remove .tar if we have bzipped or gzipped
        if ((options["gzip"] || options["bzip"]) && File.exists(tarFile))
            File.remove(tarFile);
    }
}

/************************************************************
 * gets a list of all files and subdirectories relative to the specified directory (not absolutePath)
 */
function getFileList(rootDir)
{
    var dir = new Dir(rootDir);
    dir.setCurrent();
    var rootLength = dir.absPath.length + 1; // +1 because "/" is not included in absPath
    var result = new Array();

    // add files to result
    var files = dir.entryList("*", Dir.Files | Dir.Hidden | Dir.System, Dir.Name);
    for (var f in files)
        result.push(files[f]);

    // expand dirs to absolute path
    var dirs = new Array();
    var tempDirs = dir.entryList("*", Dir.Dirs | Dir.Hidden | Dir.System, Dir.Name);
    for (var t in tempDirs) {
        if (tempDirs[t] != "." && tempDirs[t] != "..")
            dirs.push(dir.absFilePath(tempDirs[t]));
    }
    
    for (var i=0; i<dirs.length; ++i) {
         // cd to directory and add directory to result
         dir.cd(dirs[i]);
         result.push(dirs[i].right(dirs[i].length - rootLength));

        // add files
        var files = dir.entryList("*", Dir.Files | Dir.Hidden | Dir.System, Dir.Name);
        for (var f in files)
            result.push(dir.absFilePath(files[f]).right(dir.absFilePath(files[f]).length - rootLength));

        // adds subDirs to dirs
        tempDirs = dir.entryList("*", Dir.Dirs | Dir.Hidden | Dir.System, Dir.Name);
        for (var t in tempDirs) {
            if (tempDirs[t] != "." && tempDirs[t] != "..")
                dirs.push(dir.absFilePath(tempDirs[t]));
        }
    }
    return result;
}

/************************************************************
 * cleans up temp files
 */
function cleanup()
{
    // deletes distDir
    var dir = new Dir(tmpDir);
    dir.setCurrent();
    dir = new Dir(distDir);
    if (dir.exists)
        dir.rmdirs();
}


/************************************************************
 * copies some files according to platform
 */
function copyTarget(packageDir, platform)
{
    const vfbFiles = ["/src/gui/embedded/qlock.cpp",
                      "/src/gui/embedded/qlock_p.h",
                      "/src/gui/embedded/qwssignalhandler.cpp",
                      "/src/gui/embedded/qwssignalhandler_p.h",
                      "/src/gui/embedded/qvfbhdr.h"];

    if (platform != "x11")
        return;

    for (var i in vfbFiles) {
        if (!File.exists(packageDir + vfbFiles[i]))
            throw "%1 cannot be found".arg(packageDir + vfbFiles[i]);
        execute(["cp", packageDir + vfbFiles[i], packageDir + "/tools/qvfb/"]);
    }
}


/************************************************************
 * copies the special dist files according to platform and license
 * and populates the licenseHeaders array
 */
function copyDist(packageDir, platform, license)
{
    if (platform == "core")
        platform = "embedded";

    if (options["snapshots"]) 
        license = "snapshot-" + license;
    else if (options["preview"]) 
        license = "preview-" + license;

    var platformFiles = getFileList(packageDir + "/dist/" + platform);
    var licenseFiles = getFileList(packageDir + "/dist/" + license);

    //copies changes file to root
    var changesFile = packageDir + "/dist/changes-" + options["version"];
    if (File.exists(changesFile))
        execute(["cp", changesFile, packageDir]);
    
    //copies default README to root
    var readmeFile = packageDir + "/dist/README";
    if (File.exists(readmeFile))
        execute(["cp", readmeFile, packageDir]);

    // copies any platform specific files
    for (var i in platformFiles) {
        var fileName = platformFiles[i];
        var absFileName = packageDir + "/dist/" + platform + "/" + fileName;
        if (File.exists(absFileName) && File.isFile(absFileName)) {
            var dir = new Dir(new File(packageDir + "/" + fileName).path);
            if (!dir.exists)
                dir.mkdirs();
            execute(["cp", absFileName, packageDir + "/" + fileName]);
        }
    }

    // copies any license specific files
    for (var i in licenseFiles) {
        var fileName = licenseFiles[i];
        var absFileName = packageDir + "/dist/" + license + "/" + fileName;
        if (File.exists(absFileName) && File.isFile(absFileName)) {
            var dir = new Dir(new File(packageDir + "/" + fileName).path);
            if (!dir.exists)
                dir.mkdirs();
            if (fileName.startsWith("LICENSE") && (fileName.find(".") == -1))
                execute(["cp", absFileName, packageDir + "/." + fileName]);
            else
                execute(["cp", absFileName, packageDir + "/" + fileName]);

        }
    }

    // populate licenseHeaders with all files found in dist/licenses
    var licenseFiles = getFileList(packageDir + "/dist/licenses");
    for (var i in licenseFiles) {
        var fileName = licenseFiles[i];
        var absFileName = packageDir + "/dist/licenses/" + fileName;
        if (File.exists(absFileName) && File.isFile(absFileName))
            licenseHeaders[fileName] = File.read(absFileName);
    }

    //check that key files are present
    var keyFiles = ["README", "INSTALL"];
    if (!options["snapshots"])
        keyFiles.push("changes-" + options["version"]);

    if (license == "opensource") {
        keyFiles.push("LICENSE.GPL");
        keyFiles.push("LICENSE.QPL");
    } else if (license == "commercial") {
        keyFiles.push(".LICENSE");
        keyFiles.push(".LICENSE-US");
    } else if (license == "preview-opensource") {
        keyFiles.push("LICENSE.PREVIEW.OPENSOURCE");
    } else if (license == "preview-commercial") {
        keyFiles.push("LICENSE.PREVIEW.COMMERCIAL");
    } else if (license == "snapshot-commercial") {
        keyFiles.push("LICENSE.SNAPSHOT.COMMERCIAL");
    } else if (license == "snapshot-opensource") {
        keyFiles.push("LICENSE.SNAPSHOT.OPENSOURCE");
    }
    for (var i in keyFiles) {
        if (!File.exists(packageDir + "/" + keyFiles[i]))
            warning("Missing %1 in package.".arg(packageDir + "/" + keyFiles[i]));
    }
}


/************************************************************
 * returns the source files contained in the eval-patches package
 */
function getEvalFileList()
{
    evalFiles = new Array();
    evalFiles.push("/src/corelib/kernel/qtcore_eval.cpp");
    evalFiles.push("/src/corelib/eval.pri");
    evalFiles.push("/util/scripts/mac-binary/package/InstallerPane/keydec.cpp");
    evalFiles.push("/util/scripts/mac-binary/package/InstallerPane/keydec.h");
    return evalFiles;
}

/************************************************************
 * copies the directory and files needed to make eval binares from src package
 */
function copyEval(packageDir)
{
    var evalFiles = getEvalFileList();
    for (var i in evalFiles) {
        var evalFile = evalFiles[i];
        p4Copy(p4BranchPath + evalFile + "@" + p4Label,
               packageDir + evalFile);
    }
}

/************************************************************
 * runs syncqt in packageDir with the specified platform
 */
function syncqt(packageDir, platform)
{
    var dir = new Dir(packageDir);
    dir.setCurrent();
    System.setenv("QTDIR", packageDir);
    var syncqtCommand = packageDir + "/bin/syncqt";
    if (platform == "win")
        execute([syncqtCommand, "-windows"]);
    else
        execute([syncqtCommand]);
}

/************************************************************
 * runs qdoc on packageDir
 */
function qdoc(packageDir, platform, license)
{
    var dir = new Dir(packageDir);
    dir.setCurrent();
    System.setenv("QTDIR", packageDir);
    var qdocConfigFile = qdocDir;
    if (platform == "mac")
        qdocConfigFile += "/test/qt-with-xcode.qdocconf";
    else
        qdocConfigFile += "/test/qt.qdocconf";
    var qdocDefines = "-Dopensourceedition";
    if (license.find("opensource") == -1)
        qdocDefines = "-Ddesktopedition";
    if (platform == "core")
        qdocDefines = "-Dqtopiacore";

    execute([qdocCommand, qdocConfigFile, qdocDefines]);
}

/************************************************************
 * returns the default search'n'replace array for substituting tags
 */
function defaultTags(platform, license, platName)
{
    var replace = new Array();
    replace["\\$TROLLTECH\\$"] = "Trolltech ASA";
    replace["\\$THISYEAR\\$"] = startDate.getYear().toString();
    replace["\\%VERSION\\%"] = options["version"];
    if (platform == "core")
        replace["\\%PRODUCTLONG\\%"] = "Qtopia Core (Qt for embedded Linux)";
    else
        replace["\\%PRODUCTLONG\\%"] = "Qt";
    replace["#\\s*define\\s+QT_VERSION_STR\\s+\"([^\"]+)\"*"] =
        "#define QT_VERSION_STR   \"" + options["version"] + "\"";
    replace["#\\s*define\\s+QT_PACKAGEDATE_STR\\s+\"([^\"]+)\"*"] =
        "#define QT_PACKAGEDATE_STR \"" + startDate.toString().left(10) + "\"";
    replace["\\%DISTNAME\\%"] = platName;
    replace["\\*\\* \\$TROLLTECH_DUAL_LICENSE\\$\\n"] = licenseHeaders[license];
    replace["\\*\\* \\$TROLLTECH_3RDPARTY_LICENSE\\$\\n"] = licenseHeaders["3rdparty"];
    replace["\\*\\* \\$TROLLTECH_GPL_LICENSE\\$\\n"] = licenseHeaders[license];
    replace["\\*\\* \\$TROLLTECH_COMMERCIAL_LICENSE\\$\\n"] = licenseHeaders[license];

    return replace;
}

/************************************************************
 * goes through all source files (except 3rdparty) and checks for $*_LICENSE$
 */
function checkLicense(packageDir, fileList, checkMode)
{
    for (var i in fileList) {
        fileName = fileList[i];
        absFileName = packageDir + "/" + fileName;
        // skip the $QTDIR/include and 3rdparty files
        if (fileName.find(/\/3rdparty\//) == -1
            && fileName.find(/^include\//) == -1
            && fileName.find(/^doc\//) == -1
            && fileName.find(/^config.tests\//) == -1
            && fileName.find(/\.cpp$|\.h$|\.c$/) != -1) {

            content = File.read(absFileName);

            if (checkMode == "final") {
                /** FIXME, licenses were already replaced due to doc-generation **/
                if (content.find(/\*\* \$TROLLTECH_(DUAL|3RDPARTY|GPL|COMMERCIAL|INTERNAL)_LICENSE\$/) != -1) {
                    throw "%1 contains an invaid license".arg(fileName);
                }
            } else if (checkMode == "first") {
                if (content.find(/\*\* \$TROLLTECH_(DUAL|3RDPARTY|GPL|COMMERCIAL|INTERNAL)_LICENSE\$/)
                    == -1) {
                    warning(fileName + " doesn't contain a valid $TROLLTECH_*_LICENSE$ tag");
                }

                if (fileName.find(/_p.h$/) != -1
                    && content.find(/\/\/ We mean it./) == -1) {
                    warning(fileName + " doesn't contain \"We mean it\"");
                }
            } else {
                throw "Invalid mode in checkLicense";
            }
        }
    }
}

/************************************************************
 * goes through all txt files and replaces tags like %VERSION%, %THISYEAR% etc.
 */
function replaceTags(packageDir, fileList, replace)
{
    var fileName = new String();
    var absFileName = new String();
    var content = new String();
    for (var i in fileList) {
        fileName = fileList[i];
        absFileName = packageDir + "/" + fileName;
        if (File.isFile(absFileName) && !isBinaryFile(absFileName)) {
            //only replace for non binary files
            content = File.read(absFileName);
            if (content.find(/[\x80-\xff]/) != -1) {
                warning("Found a high-byte non-ascii character in " + fileName);
            }
            for (var i in replace) {
                var re = new RegExp(i);
		re.global = true;
                content = content.replace(re, replace[i]);
            }
            // special case for $MODULE$
            if (content.find(/\$MODULE\$/) != -1) {
                var match = false;
                for (var i in moduleMap) {
                    if (fileName.find(moduleMap[i]) != -1) {
                        content = content.replace(/\$MODULE\$/, i);
                        match = true;
                        break;
                    }
                }
                if (!match)
                    warning("No module map for: " + fileName);
            }
            File.write(absFileName, content);
        }
    }
}

/************************************************************
 * checks out depotFile to localFile
 * 
 * typically used when files are needed from the depot that can have
 * been purged from the package (like qdoc configurations and scripts
 * etc.)
 */
function p4Copy(depotFile, localFile)
{
    execute([p4Command, "print", "-o", localFile, "-q", depotFile]);
    if (!File.exists(localFile))
        throw "Failed copying file: %1 to: %2".arg(depotFile).arg(localFile);
    execute(["chmod", "ug+w", localFile]);
    return localFile;
}

/************************************************************
 * prints out text with indentation
 */
function print(text)
{
    var i = indentation;
    var spaces = new String();
    while (i--)
        spaces += ' ';
    System.println(spaces + text);
}

/************************************************************
 * prints out warning text with indentation
 */
function warning(text)
{
    print("** Warning! ** " + text);
}

/************************************************************
 * returns true if the file exists, is a file, is executable or has a binary extension
 */
function isBinaryFile(fileName)
{
    if (File.exists(fileName) && File.isFile(fileName)) {
        for (var r in binaryFileList)
            if (fileName.find(binaryFileList[r]) != -1)
                return true;
        var file = new File(fileName);
        if (file.executable) {
            file.open(File.ReadOnly);
            var isScript = file.readLine().lower().startsWith("#!");
            file.close();
            if (isScript)
                return false;
            return true;
        } else {
            for (var i in binaryExtensions)
                if (file.extension.lower() == binaryExtensions[i])
                    return true;
        }
    }
    return false;
}

/************************************************************
 * runs the command and prints out stderror if not empty
 */
function execute(command, stdin) {
    var start = Date().getTime();
    var error = Process.execute(command, stdin);
    var runTime = Math.floor((Date().getTime() - start)/1000);
    if (runTime > 0)
        print("%1\n   ->took %1 second(s)".arg(command).arg(runTime));
    if (error != 0) {
        if (Process.stderr.length > 0)        
            File.write(outputDir + "/error.log", Process.stderr);
        throw "Error runnning: %1 stderr: %2".arg(command).arg(Process.stderr.left(1000));
    } else if (Process.stderr.length > 0
               && Process.stderr.left(200).lower().find(/warning|error/) != -1) {
        warning("Running %1 stderr: %2".arg(command).arg(Process.stderr.left(200)));
    }
}
