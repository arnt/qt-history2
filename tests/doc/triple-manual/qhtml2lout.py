#!/usr/bin/env python

import os
import re
import string
import sys


ANAME_RE = re.compile(r'<a name=([^>]+)>(<"?/"?a>)?')
AHREF_RE = re.compile(r'<a href=[^>]+>(.*?)<"?/"?a>')
H_RE = re.compile(r'<h([1-5])[^>]*>(.*?)<"?/"?h[1-5]>')
PRE_RE = re.compile(r'<pre>(.*?)<"?/"?pre>', re.DOTALL)
B_RE = re.compile(r'<b>(.*?)<"?/"?b>')
I_RE = re.compile(r'<(?:em|i)>(.*?)<"?/"?(?:em|i)>')
COMMENTS_RE = re.compile(r'<!--.*?-->', re.DOTALL)
TT_RE = re.compile(r'<(?:tt|code)>(.*?)<"?/"?(?:tt|code)>')
P_RE = re.compile(r'<p[^>]*>')
IMG_RE = re.compile(r'<img.*?src=\x02([^\x02]+)\x02.*?>')
OL_RE = re.compile(r'<ol[^>]*>')
UL_RE = re.compile(r'<ul[^>]*>')
LOUT_RE = re.compile(r'@[A-Z]\w*\{([^}]+)\}')
WRAP_RE = re.compile(r'(@PP\n|@LI{\n)(.*)')
HTML_ENTITY_RE = re.compile(r'&#?(\w+);')
CLASS_RE = re.compile(r'@B\{(Q[A-Z]\w+)\}')
BARE_CLASS_RE = re.compile(r'Q[A-Z]\w+')
INDEX_RE = re.compile(r'<!-- index([^>]+)-->')
FIX_INDEX_RE = re.compile(r'(@(?:Numbered|Bullet)List\n)(.*?)(@LI)', re.DOTALL)
FIX_FIGURE_RE = re.compile(
		    r'(@Figure)'
		    r'([^{}]*?@IncludeGraphic[^}]*?\})'
		    r'\s*\x03@QD\{(.+?)\}\x04'
		    , re.DOTALL)

ENTITY_TO_LOUT = { 
      '60': '<',
      '62': '>',
      '38': '"&"',
      '92': '"\\\\"',
     '163': '{@Sterling}',
     '165': '{@Yen}',
    '8212': '---',
    '8364': '{@Euro}',
    '8482': '{@TradeMark}',

    'amp':	'"&"',		       # &  ampersand
    'lt':	'<',		       # <  less than
    'gt':	'>',		       # >  greater than
    'quot':	'"\\""',	       # "  

    'AElig':	'{@Char AE}',          # Æ  capital AE diphthong (ligature)
    'Aacute':	'{@Char Aacute}',      # Á  capital A, acute accent
    'Acirc':	'{@Char Acircumflex}', # Â  capital A, circumflex accent
    'Agrave':	'{@Char Agrave}',      # À  capital A, grave accent
    'Aring':	'{@Char Aring}',       # Å  capital A, ring
    'Atilde':	'{@Char Atilde}',      # Ã  capital A, tilde
    'Auml':	'{@Char Adieresis}',   # Ä  capital A, dieresis or umlaut mark
    'Ccedil':	'{@Char Ccedilla}',    # Ç  capital C, cedilla
    'ETH':	'{@Char Eth}',         # Ð  capital Eth, Icelandic
    'Eacute':	'{@Char Eacute}',      # É  capital E, acute accent
    'Ecirc':	'{@Char Ecircumflex}', # Ê  capital E, circumflex accent
    'Egrave':	'{@Char Egrave}',      # È  capital E, grave accent
    'Euml':	'{@Char Edieresis}',   # Ë  capital E, dieresis or umlaut mark
    'Iacute':	'{@Char Iacute}',      # Í  capital I, acute accent
    'Icirc':	'{@Char Icircumflex}', # Î  capital I, circumflex accent
    'Igrave':	'{@Char Igrave}',      # Ì  capital I, grave accent
    'Iuml':	'{@Char Idieresis}',   # Ï  capital I, dieresis or umlaut mark
    'Ntilde':	'{@Char Ntilde}',      # Ñ  capital N, tilde
    'Oacute':	'{@Char Oacute}',      # Ó  capital O, acute accent
    'Ocirc':	'{@Char Ocircumflex}', # Ô  capital O, circumflex accent
    'Ograve':	'{@Char Ograve}',      # Ò  capital O, grave accent
    'Oslash':	'{@Char Oslash}',      # Ø  capital O, slash
    'Otilde':	'{@Char Otilde}',      # Õ  capital O, tilde
    'Ouml':	'{@Char Odieresis}',   # Ö  capital O, dieresis or umlaut mark
    'THORN':	'{@Char Thorn}',       # Þ  capital THORN, Icelandic
    'Uacute':	'{@Char Uacute}',      # Ú  capital U, acute accent
    'Ucirc':	'{@Char Ucircumflex}', # Û  capital U, circumflex accent
    'Ugrave':	'{@Char Ugrave}',      # Ù  capital U, grave accent
    'Uuml':	'{@Char Udieresis}',   # Ü  capital U, dieresis or umlaut mark
    'Yacute':	'{@Char Yacute}',      # Ý  capital Y, acute accent
    'aacute':	'{@Char aacute}',      # á  small a, acute accent
    'acirc':	'{@Char acircumflex}', # â  small a, circumflex accent
    'aelig':	'{@Char ae}',          # æ  small ae diphthong (ligature)
    'agrave':	'{@Char agrave}',      # à  small a, grave accent
    'aring':	'{@Char aring}',       # å  small a, ring
    'atilde':	'{@Char atilde}',      # ã  small a, tilde
    'auml':	'{@Char adieresis}',   # ä  small a, dieresis or umlaut mark
    'ccedil':	'{@Char ccedilla}',    # ç  small c, cedilla
    'eacute':	'{@Char eacute}',      # é  small e, acute accent
    'ecirc':	'{@Char ecircumflex}', # ê  small e, circumflex accent
    'egrave':	'{@Char egrave}',      # è  small e, grave accent
    'eth':	'{@Char eth}',         # ð  small eth, Icelandic
    'euml':	'{@Char edieresis}',   # ë  small e, dieresis or umlaut mark
    'iacute':	'{@Char iacute}',      # í  small i, acute accent
    'icirc':	'{@Char icircumflex}', # î  small i, circumflex accent
    'igrave':	'{@Char igrave}',      # ì  small i, grave accent
    'iuml':	'{@Char idieresis}',   # ï  small i, dieresis or umlaut mark
    'ntilde':	'{@Char ntilde}',      # ñ  small n, tilde
    'oacute':	'{@Char oacute}',      # ó  small o, acute accent
    'ocirc':	'{@Char ocircumflex}', # ô  small o, circumflex accent
    'ograve':	'{@Char ograve}',      # ò  small o, grave accent
    'oslash':	'{@Char oslash}',      # ø  small o, slash
    'otilde':	'{@Char otilde}',      # õ  small o, tilde
    'ouml':	'{@Char odieresis}',   # ö  small o, dieresis or umlaut mark
    'szlig':	'{@Char germandbls}',  # ß  small sharp s, German (sz ligature)
    'thorn':	'{@Char thorn}',       # þ  small thorn, Icelandic
    'uacute':	'{@Char uacute}',      # ú  small u, acute accent
    'ucirc':	'{@Char ucircumflex}', # û  small u, circumflex accent
    'ugrave':	'{@Char ugrave}',      # ù  small u, grave accent
    'uuml':	'{@Char udieresis}',   # ü  small u, dieresis or umlaut mark
    'yacute':	'{@Char yacute}',      # ý  small y, acute accent
    'yuml':	'{@Char ydieresis}',   # ÿ  small y, dieresis or umlaut mark

    # Some extra Latin 1 chars that are listed in the HTML3.2 draft 1996/05/21
    'copy':	'{@CopyRight}',        # ©  copyright sign
    'reg':	'{@Register}',         # ®  registered sign
    'nbsp':	'~',                   #    non breaking space

    # Additional ISO-8859/1 entities listed in rfc1866 (section 14)
    'iexcl':	'{@Char exclamdown}',       # ¡
    'cent':	'{@Char cent}',             # ¢
    'pound':	'{@Sterling}',              # £
    'curren':	'{@Char currency}',         # ¤
    'yen':	'{@Yen}',                   # ¥
    'brvbar':	'{@Char bar}',              # ¦
    'sect':	'{@SectSym}',               # §
    'uml':	'{@Char dieresis}',         # ¨
    'ordf':	'{@Char ordfeminine}',      # ª
    'laquo':	'{@Char guillemotleft}',    # «
    'not':	'{@Char logicalnot}',       # ¬    
    'shy':	'{@Char hyphen}',           # ­
    'macr':	'{@Char macron}',           # ¯
    'deg':	'{@Char degree}',           # °
    'plusmn':	'{@Char plusminus}',        # ±
    'sup1':	'{@Char onesuperior}',      # ¹
    'sup2':	'{@Char twosuperior}',      # ²
    'sup3':	'{@Char threesuperior}',    # ³
    'acute':	'{@Char acute}',            # ´
    'micro':	'{@Char mu}',               # µ
    'para':	'{@ParSym}',                # ¶
    'middot':	'{@Char periodcentered}',   # ·
    'cedil':	'{@Char cedilla}',          # ¸
    'ordm':	'{@Char ordmasculine}',     # º
    'raquo':	'{@Char guillemotright}',   # »
    'frac14':	'{@Char onequarter}',       # ¼
    'frac12':	'{@Char onehalf}',          # ½
    'frac34':	'{@Char threequarters}',    # ¾
    'iquest':	'{@Char questiondown}',     # ¿
    'times':	'{@Multiply}',              # ×    
    'divide':	'{@Divide}',                # ÷
    }

PART_STRING = '\n    @PartNumber { Part %s }\n    @PartTitle { %s }'


def image(match):
    file = match.group(1)
    epsfile = 'images/' + file[:file.rfind('.')] + '.eps'
    if not os.access(epsfile + '.gz', os.F_OK):
	os.system('convert $QTDIR/doc/html/%s %s' % (file, epsfile))
	os.system(r"perl -i -pe's/^\%%\%%LanguageLevel: 1//' %s" % epsfile)
	os.system('gzip -9 %s' % epsfile)
    return '@Figure 0.3 @Scale @IncludeGraphic { \x01%s.gz\x01 }' % epsfile


def href(match):
    text = match.group(1)
    if text[0] == 'Q' and text.find(' ') == -1:
	return '@B{%s}' % text
    else:
	return text
#	# We do the following in wrap()
#	link = normalise(text, 1)
#	if link:
#	    return '%s [p.~@PageOf{%s}]' % (text, link)
#	else:
#	    return text


def entity(match):
    text = match.group(1)
    if ENTITY_TO_LOUT.has_key(text):
	return ENTITY_TO_LOUT[text]
    else:
	return text


def wrap(match):
    def indexclass(match):
	klass = match.group(1)
	return '@B{%s}%s @Index{ %s }' % (klass, canonicalise(klass), klass)
	
    leader = match.group(1)
    words = match.group(2).split(' ')
    text = ''
    count = 0
    for word in words:
	text += word
	count += len(word)
	if count < 60:
	    text += ' '
	    count += 1
	else:
	    text += '\n'
	    count = 0
    if text[-1] == ' ':
	text = text[:-1] + '\n'
    text = text.replace('\n\n', '\n')
    for target in TARGETS.keys():
	text = text.replace(target,
			    '%s [p.~@PageOf{%s}]' % (target, TARGETS[target]))
    text = CLASS_RE.sub(indexclass, text)
    i = 0
    while 1:
	i = text.find('@Verbatim{', i)
	if i == -1:
	    break
	j = text.find('}', i)
	block = text[i:j].replace('\n', ' ')
	text = text[:i] + block + text[j:]
	i = j
    return leader + text
    

LINKS = {}
TARGETS = {}

def canonicalise(s):
    result = re.sub(r'<[^>]+>', '', s.strip().lower())
    result = re.sub(r'[^a-z]+', '.', result)
    while result[0] == '.':
	result = result[1:]
    while result[-1] == '.':
	result = result[:-1]
    return result


def normalise(s, lookup=0):
    result = canonicalise(s)
    if not lookup:
	key = s.strip()
	if ' ' in key and key[0] in string.uppercase:
	    TARGETS[key] = result
	while LINKS.has_key(result):
	    result += 'x'
	LINKS[result] = 1
	return result
    else:
	if LINKS.has_key(result):
	    return result
	else:
	    return None


def h(match):
    level = match.group(1)
    title = match.group(2)
    tag = normalise(title)
    if level == '1':
	return '@Chapter\n    @Title { %s }\n    @Tag { %s }\n@Begin' % (
	       title, tag)
    elif level == '2':
	return '''@End @Section\n@Section\n    @Title { %s }\n\
    @Tag { %s }\n@Begin''' % (title, tag)
    elif level == '3':
	return '@LD @Heading { %s{@PageMark{%s}} }' % (title, tag)
    else:
	return '@LD @I { %s{@PageMark{%s}} }' % (title, tag)


def pre(match):
    body = match.group(1)
    classes = BARE_CLASS_RE.findall(body)
    indexentries = ''
    seen = {}
    for klass in classes:
	if not seen.has_key(klass):
	    seen[klass] = 1
	    indexentries += '\n%s @Index{ %s }' % (canonicalise(klass), klass)
    return '''@LD { 0.75 1.00 } @Scale \
@F @RawVerbatim @Begin\n%s@End @RawVerbatim%s''' % (body, indexentries)


def indexentry(match):
    elements = match.group(1).strip().split('!')
    if not elements[0]:
	return ''
    key = canonicalise(elements[0])
    if len(elements) == 1:
	return '\n%s @Index{ %s }' % (key, elements[0])
    else:
	raw = '\n%s @RawIndex{ %s }' % (key, elements[0])
	if len(elements) == 2:
	    rest = ' '.join(elements[1:])
	    return raw + '\n%s.%s @SubIndex{ %s }' % (key, canonicalise(rest), rest)
	else:
	    key1 = canonicalise(elements[1])
	    subraw = '\n%s @RawSubIndex{ %s }' % (key1, elements[1])
	    rest = ' '.join(elements[2:])
	    return raw + '\n%s.%s @SubSubIndex{ %s }' % (key1, canonicalise(rest), rest)




def process(file):
    fh = open(file)
    data = fh.read()
    fh.close()

    data = data.replace('"', '\x02')
    data = data.replace('/', '"/"')
    data = data.replace('|', '"|"')
    data = data.replace('@', '"@"')
    data = HTML_ENTITY_RE.sub(entity, data)
    data = data.replace('#', '"#"')
    data = data[data.find('<h1'):]
    data = data[:data.find('<!-- eof')]
    data = ANAME_RE.sub('', data)
    data = AHREF_RE.sub(href, data)
    data = H_RE.sub(h, data)
    data = B_RE.sub(r'@B{\1}', data)
    data = I_RE.sub(r'@I{\1}', data)
    data = TT_RE.sub(r'@F @Verbatim{\1}', data)
    data = PRE_RE.sub(pre, data) 
    data = P_RE.sub('@PP\n', data)
    data = IMG_RE.sub(image, data)
    data = OL_RE.sub('@BulletList\n', data)
    data = UL_RE.sub('@NumberedList\n', data)
    data = INDEX_RE.sub(indexentry, data)
    data = COMMENTS_RE.sub('', data)
    data = data.replace('<br>', '@LLP\n')
    data = data.replace('<"/"p>', '')
    data = data.replace('<"/"ul>', '}\n@EndList\n')
    data = data.replace('<"/"ol>', '}\n@EndList\n')
    data = data.replace('<li>', '}\n@LI{')
    data = data.replace('<blockquote>', '\x03@QD{')
    data = data.replace('<"/"blockquote>', '}\x04')
    data = data.replace('@BeginSections\n@End @Section', '@BeginSections')
    data = data.replace('@ID{@PP', '@ID{')
    data = data.replace('@QD{@PP', '@QD{')
    data = data.replace('@QD{\n@PP', '@QD{')
    data = data.replace('@LI{@PP', '@LI{')
    data = data.replace('@BulletList\n}', '@BulletList')
    data = data.replace('@NumberedList\n}', '@NumberedList')
    data = data.replace('@Begin\n@End @Section', '@Begin\n@BeginSections')
    data = WRAP_RE.sub(wrap, data)
    data = data.replace('\n @PP', '\n@PP')
    data = data.replace('\x01', '"')
    data = data.replace('\x02', r'"\""')

    i = 0
    while 1:
	i = data.find('@NumberedList\n', i)
	if i == -1:
	    i = data.find('@BulletList\n', i)
	    if i == -1:
		break
	j = data.find('@LI', i)
	if data[j - 2] == '}':
	    data = data[:j - 2] + data[j - 1:]
	i = j

    i = data.find('@Chapter')
    if file == 'assistant-1.html':
	data = data[:i + 8] + PART_STRING % ('I', 'Qt Assistant') + data[i + 8:]
    elif file == 'designer-manual-1.html':
	data = data[:i + 8] + PART_STRING % ('II', 'Qt Designer') + data[i + 8:]
    elif file == 'linguist-manual-1.html':
	data = data[:i + 8] + PART_STRING % (
			      'III', "Qt's Translation Tools") + data[i + 8:]

    def fixverbatim(block):
	block = LOUT_RE.sub(r'\1', block)
	block = block.replace('"@"', '@')
	block = block.replace('"#"', '#')
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
    data = data.replace('@BeginSections\n@EndSections', '')
    data = data.replace('\n}\n@LI{', '}\n@LI{')
    data = data.replace('\n}\n@EndList', '}\n@EndList')
    data = FIX_INDEX_RE.sub(r'\2\1\3', data)
    data = FIX_FIGURE_RE.sub(r'\1\n    @Caption { \3 }\n\2', data)
    data = data.replace('\x03', '')
    data = data.replace('\x04', '')

    fh = open(file[file.rfind('/') + 1:file.rfind('.') + 1] + 'lout', 'w')
    fh.write(data)
    fh.close()
    

def main():
    for file in sys.argv[1:]:
	process(file)


if __name__ == '__main__':
    main()
