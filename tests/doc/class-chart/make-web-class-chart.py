#!/usr/bin/env python

import getopt
import glob
import os
import popen2
import sys
import zipfile

try:
    import Image
    import ImageEnhance

except ImportError:

    print "You need to install the Python Imaging Library (PIL) before you"
    print "can use this utility. This can be downloaded from"
    print
    print "http://www.pythonware.com/library/"
    print

__version__ = "1.0.0"


scale = 0.5
brightness = 1
verbose = 0

def find_files(path, pattern):

    found = []
    
    # Look for matching files.
    
    objects = glob.glob(os.path.join(path, pattern))
    
    for object_path in objects:
    
        if os.path.isfile(object_path):
        
            found.append( (object_path, os.path.split(object_path)[-1]) )
    
    # Look for directories.
    
    for obj in os.listdir(path):
    
        file_path = os.path.join(path, obj)

        if os.path.isdir(file_path):
        
            found = found + find_files(file_path, pattern)
    
    return found


def process(path, scale, brightness):

    image = Image.open(path)
    if image.size[0] < image.size[1]:
    
        # Rotate the image.
        image = image.rotate(-90)
    
    if scale != 1:
    
        image = image.resize(
            (int(image.size[0] * scale), int(image.size[1] * scale)),
            Image.ANTIALIAS
            )
    
    if brightness != 1:
    
        image = ImageEnhance.Brightness(image)
        image.enhance(brightness)
    
    # Crop the image.
    image = image.crop((22,38,1155,827))
    
    # Reduce the number of colours in the image.
    # The image will end up with more than four colours, but we have to try...
    image = image.convert("P", dither=0, palette=0, colors=4)
    image.save(path, optimize = 1)


def make_output_name(filename):

    at = filename.rfind(".pdf")
    
    if at != -1:
        return filename[:at]+".png"
    else:
        return None
    


def usage():
    print """\
usage: make-web-class-chart.py [option] <output directory>

Look for files matching the search pattern beneath the search path, and write
PDF files to the output directory. The search pattern can contain wildcards,
but you must remember to quote the pattern in the shell.

-b --brightness <float> Brighten the image using the factor given.
                        Default is %0.2f. Values less than 1.0 cause the
                        image to be darkened.
-h --help
-s --scale <float> The scaling factor to apply. Default is %0.2f.
-v --verbose

make-web-class-chart.py %s depends on ghostscript and the Python Imaging Library.
""" % (brightness, scale, __version__)
    sys.exit()

if __name__ == "__main__":

    try:
        opts, args = getopt.getopt(sys.argv[1:], "vhs:b:",
                                   ["verbose", "help", "scale=", "brightness="])
    except getopt.GetoptError:
        usage()
    for option, value in opts:
        if option in ("-s", "--scale"):
            scale = float(value)
        if option in ("-b", "--brightness"):
            brightness = float(value)
        if option in ("-v", "--verbose"):
            verbose = 1
    if len(args) != 1:
        usage()
    
    output_dir = args[0]
    
    files = find_files(os.path.split(sys.argv[0])[0], "qt33-class-chart.pdf")
    
    if files == []:
    
        print "No files found."
        sys.exit()
    
    if verbose:
    
        print "Found the following files:"
        for file_path, filename in files:
        
            print file_path
    
    if not os.path.exists(output_dir):
    
        try:
        
            os.mkdir(output_dir)
        
        except OSError:
        
            print "Failed to create the output directory: %s" % output_dir
            sys.exit()
    
    temp_dir = os.path.join(output_dir, "temp")

    if not os.path.exists(temp_dir):
    
        try:
        
            os.mkdir(temp_dir)
        
        except OSError:
        
            print "Failed to create the temporary directory: %s" % temp_dir
            sys.exit()
    
    zipfiles = filter(lambda (path, name): name[-4:] == ".zip", files)
    files = filter(lambda (path, name): name[-4:] != ".zip", files)
    
    for file_path, filename in zipfiles:
    
        docs = zipfile.ZipFile(file_path)
        
        filenames = filter(
            lambda name: name[-4:] == ".pdf",
            map(lambda info: info.filename, docs.filelist)
            )
        
        for filename in filenames:

            # Remove any leading path from the filename.
            doc_filename = os.path.split(filename)[-1]

            if verbose:
                print "Extracting %s as %s" % (filename, doc_filename)

            temp_uncompressed_path = os.path.join(temp_dir, doc_filename)
            try:

                open(temp_uncompressed_path, "wb").write(
                    docs.read(filename)
                    )
                files.append( (temp_uncompressed_path, doc_filename) )

            except IOError:

                print "Failed to extract %s from %s" % (
                    filename, file_path
                    )
    
    for file_path, filename in files:
    
        output_name = make_output_name(filename)
        
        if output_name is None:
            continue
        
        if verbose:
            print "Creating %s from %s" % (output_name, file_path)
        
        temp_path = os.path.join(temp_dir, output_name)
        
        if os.path.exists(temp_path):
        
            q = raw_input("%s already exists. Overwrite? [y/N]" % output_name)
            if q != "y":
            
                continue
        
        if filename[-3:] == ".gz":
        
            temp_compressed_path = os.path.join(temp_dir, filename)
            os.system("cp %s %s" % (file_path, temp_compressed_path))
            os.system("gzip -df %s" % temp_compressed_path)
            os.system("gs -dNOPAUSE -dBATCH -dQUIET -sDEVICE=png16m "
                      "-dFirstPage=1 -dLastPage=1 "
                      "-sOutputFile=%s -r144 %s >& /dev/null" % (
                      temp_path, temp_compressed_path[:-3]))
        
        elif filename[-4:] == ".pdf":
        
            os.system("gs -dNOPAUSE -dBATCH -dQUIET -sDEVICE=png16m "
                  "-dFirstPage=1 -dLastPage=1 "
                  "-sOutputFile=%s -r144 %s >& /dev/null" % (
                  temp_path, file_path))
        
        process(temp_path, scale, brightness)

        if verbose:
            print "Wrote %s" % output_name
    
    so, si, se = popen2.popen3("pngcrush -d %s %s" % (
        output_dir, os.path.join(temp_dir, "*")
        ))
    si.close()
    
    if se.read():
    
        # There were problems with pngcrush. Move the files into the output
        # directory.
        os.system("mv %s %s" % (os.path.join(temp_dir, "*"), output_dir))
    
    os.system("rm -rf %s" % temp_dir)
    
    sys.exit()
