#!/usr/bin/env python

import os
import re
import sys


ANAME_RE = re.compile(r'<a name=[^>]+>(<"/"a>)?')
AHREF_RE = re.compile(r'<a href=[^>]+>(.*?)<"/"a>')
H1_RE = re.compile(r'<h1[^>]*>(.*?)<"/"h1>')
H2_RE = re.compile(r'<h2[^>]*>(.*?)<"/"h2>')
H3_RE = re.compile(r'<h3[^>]*>(.*?)<"/"h3>')
H4_H5_RE = re.compile(r'<h[45][^>]*>(.*?)<"/"h[45]>')
PRE_RE = re.compile(r'<pre>(.*?)<"/"pre>', re.DOTALL)
B_RE = re.compile(r'<b>(.*?)<"/"b>')
I_RE = re.compile(r'<(?:em|i)>(.*?)<"/"(?:em|i)>')
COMMENTS_RE = re.compile(r'<!--.*?-->', re.DOTALL)
TT_RE = re.compile(r'<(?:tt|code)>(.*?)<"/"(?:tt|code)>')
P_RE = re.compile(r'<p[^>]*>')
IMG_RE = re.compile(r'<img.*?src=\x02([^\x02]+)\x02.*?>')
OL_RE = re.compile(r'<ol[^>]*>')
UL_RE = re.compile(r'<ul[^>]*>')
LOUT_RE = re.compile(r'@[A-Z]\w*\{([^}]+)\}')


def image(match):
    file = match.group(1)
    epsfile = 'images/' + file[:file.rfind('.')] + '.eps'
    if not os.access(epsfile + '.gz', os.F_OK):
	#print 'convert $QTDIR/qt/doc/html/%s %s' % (file, epsfile)
	os.system('convert $QTDIR/doc/html/%s %s' % (file, epsfile))
	os.system('gzip -9 %s' % epsfile)
    return '@Figure @Location { AfterLine } ' +\
	   '0.3 @Scale @IncludeGraphic { \x01%s.gz\x01 }' % epsfile


def href(match):
    text = match.group(1)
    if text[0] == 'Q' and text.find(' ') == -1:
	return '@B{%s}' % text
    else:
	return text


def process(file):
    fh = open(file)
    data = fh.read()
    fh.close()

    data = data.replace('"', '\x02')
    data = data.replace('/', '"/"')
    data = data.replace('|', '"|"')
    data = data.replace('@', '"@"')
    data = data.replace('&amp;', '"&"')
    data = data.replace('&lt;', '<')
    data = data.replace('&gt;', '>')
    data = data[data.find('<h1'):]
    data = data[:data.find('<!-- eof')]
    data = ANAME_RE.sub('', data)
    data = AHREF_RE.sub(href, data)
    data = H1_RE.sub(r'@Chapter\n    @Title { \1 }\n@Begin', data)
    data = H2_RE.sub(r'@End @Section\n@Section\n    @Title { \1 }\n@Begin', data)
    data = H3_RE.sub(r'@LD @Heading { \1 }', data)
    data = H4_H5_RE.sub(r'@LD @I { \1 }', data)
    data = B_RE.sub(r'@B{\1}', data)
    data = I_RE.sub(r'@I{\1}', data)
    data = TT_RE.sub(r'@F @Verbatim{\1}', data)
    data = PRE_RE.sub(r'@LD @F @RawVerbatim @Begin\n\1@End @RawVerbatim', data)
    data = P_RE.sub('@PP\n', data)
    data = COMMENTS_RE.sub('', data)
    data = IMG_RE.sub(image, data)
    data = OL_RE.sub('@BulletList\n', data)
    data = UL_RE.sub('@NumberedList\n', data)
    data = data.replace('<br>', '@LLP\n')
    data = data.replace('<"/"p>', '')
    data = data.replace('<"/"ul>', '}\n@EndList\n')
    data = data.replace('<"/"ol>', '}\n@EndList\n')
    data = data.replace('<li>', '}\n@LI{')
    data = data.replace('<blockquote>', '@QD{')
    data = data.replace('<"/"blockquote>', '}')
    data = data.replace('@BeginSections\n@End @Section', '@BeginSections')
    data = data.replace('@ID{@PP', '@ID{')
    data = data.replace('@LI{@PP', '@LI{')
    data = data.replace('@BulletList\n}', '@BulletList gap { 0.25v }')
    data = data.replace('@NumberedList\n}', '@NumberedList gap { 0.25v }')
    data = data.replace('@Begin\n@End @Section', '@Begin\n@BeginSections')
    data = data.replace('\x01', '"')
    data = data.replace('\x02', r'"\""')

    def fixverbatim(block):
	block = LOUT_RE.sub(r'\1', block)
	block = block.replace('"@"', '@')
	block = block.replace('"&"', '&')
	block = block.replace('"/"', '/')
	block = block.replace('"|"', '|')
	block = block.replace(r'"\""', '"')
	return block

    i = 0
    while 1:
	i = data.find('@RawVerbatim', i)
	if i == -1:
	    break
	if i > 4 and data[i - 5:i - 1] == '@End':
	    i += 20
	    continue
	if data[i + 12] == '{':
	    i += 13
	    j = data.find('}', i)
	else:
	    i += 20
	    j = data.find('@End @RawVerbatim', i)
	data = data[:i] + fixverbatim(data[i:j]) + data[j:]
	i = j

    i = 0
    while 1:
	i = data.find('@Verbatim', i)
	if i == -1:
	    break
	if i > 4 and data[i - 5:i - 1] == '@End':
	    i += 17
	    continue
	if data[i + 9] == '{':
	    i += 10
	    j = data.find('}', i)
	else:
	    i += 17
	    j = data.find('@End @Verbatim', i)
	data = data[:i] + fixverbatim(data[i:j]) + data[j:]
	i = j

    data += '@End @Section\n@EndSections\n@End @Chapter\n'
    i = data.find('@End @Section')
    j = data.find('@Section')
    k = data.find('@BeginSections')
    if i < j:
	x = ''
	if k == -1:
	    x = '@BeginSections'
	data = data[:i] + x + data[i + 13:]

    fh = open(file[file.rfind('/') + 1:file.rfind('.') + 1] + 'lout', 'w')
    fh.write(data)
    fh.close()
    

def main():
    for file in sys.argv[1:]:
	process(file)


if __name__ == '__main__':
    main()
