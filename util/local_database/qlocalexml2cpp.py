#! /usr/bin/python

import sys
import xml.dom.minidom

def firstChildElt(parent, name):
    child = parent.firstChild
    while child:
        if child.nodeType == parent.ELEMENT_NODE \
            and (not name or child.nodeName == name):
            return child
        child = child.nextSibling
    return False

def nextSiblingElt(sibling, name):
    sib = sibling.nextSibling
    while sib:
        if sib.nodeType == sibling.ELEMENT_NODE \
            and (not name or sib.nodeName == name):
            return sib
        sib = sib.nextSibling
    return False

def eltText(elt):
    result = ""
    child = elt.firstChild
    while child:
        if child.nodeType == elt.TEXT_NODE:
            if result:
                result += " "
            result += child.nodeValue
        child = child.nextSibling
    return result

def loadLanguageMap(doc):
    result = {}

    language_list_elt = firstChildElt(doc.documentElement, "languageList")
    language_elt = firstChildElt(language_list_elt, "language")
    while language_elt:
        language_id = int(eltText(firstChildElt(language_elt, "id")))
        language_name = eltText(firstChildElt(language_elt, "name"))
        language_code = eltText(firstChildElt(language_elt, "code"))
        result[language_id] = (language_name, language_code)
        language_elt = nextSiblingElt(language_elt, "language")

    return result

def loadCountryMap(doc):
    result = {}

    country_list_elt = firstChildElt(doc.documentElement, "countryList")
    country_elt = firstChildElt(country_list_elt, "country")
    while country_elt:
        country_id = int(eltText(firstChildElt(country_elt, "id")))
        country_name = eltText(firstChildElt(country_elt, "name"))
        country_code = eltText(firstChildElt(country_elt, "code"))
        result[country_id] = (country_name, country_code)
        country_elt = nextSiblingElt(country_elt, "country")

    return result

def loadDefaultMap(doc):
    result = {}

    list_elt = firstChildElt(doc.documentElement, "defaultCountryList")
    elt = firstChildElt(list_elt, "defaultCountry")
    while elt:
        country = eltText(firstChildElt(elt, "country"));
        language = eltText(firstChildElt(elt, "language"));
        result[language] = country;
        elt = nextSiblingElt(elt, "defaultCountry");
    return result

def fixedCountryName(name, dupes):
    if name in dupes:
        return name + "Country"
    return name

def fixedLanguageName(name, dupes):
    if name in dupes:
        return name + "Language"
    return name

def findDupes(country_map, language_map):
    country_set = set([ v[0] for a, v in country_map.iteritems() ])
    language_set = set([ v[0] for a, v in language_map.iteritems() ])
    return country_set & language_set

def languageNameToId(name, language_map):
    for key in language_map.keys():
        if language_map[key][0] == name:
            return key
    return -1

def countryNameToId(name, country_map):
    for key in country_map.keys():
        if country_map[key][0] == name:
            return key
    return -1

def convertFormat(format):
    result = ""
    i = 0
    while i < len(format):
        if format[i] == "'":
            result += "'"
            i += 1
            while i < len(format) and format[i] != "'":
                result += format[i]
                i += 1
            if i < len(format):
                result += "'"
                i += 1
        else:
            s = format[i:]
            if s.startswith("EEEE"):
                result += "dddd"
                i += 4
            elif s.startswith("EEE"):
                result += "ddd"
                i += 3
            elif s.startswith("a"):
                result += "AP"
                i += 1
            elif s.startswith("z"):
                result += "t"
                i += 1
            elif s.startswith("v"):
                i += 1
            else:
                result += format[i]
                i += 1

    return result

class Locale:
    def __init__(self, elt):
        self.language = eltText(firstChildElt(elt, "language"))
        self.country = eltText(firstChildElt(elt, "country"))
        self.decimal = int(eltText(firstChildElt(elt, "decimal")))
        self.group = int(eltText(firstChildElt(elt, "group")))
        self.listDelim = int(eltText(firstChildElt(elt, "list")))
        self.percent = int(eltText(firstChildElt(elt, "percent")))
        self.zero = int(eltText(firstChildElt(elt, "zero")))
        self.minus = int(eltText(firstChildElt(elt, "minus")))
        self.exp = int(eltText(firstChildElt(elt, "exp")))
        self.longDateFormat = convertFormat(eltText(firstChildElt(elt, "longDateFormat")))
        self.shortDateFormat = convertFormat(eltText(firstChildElt(elt, "shortDateFormat")))
        self.longTimeFormat = convertFormat(eltText(firstChildElt(elt, "longTimeFormat")))
        self.shortTimeFormat = convertFormat(eltText(firstChildElt(elt, "shortTimeFormat")))
        self.longMonths = eltText(firstChildElt(elt, "longMonths"))
        self.shortMonths = eltText(firstChildElt(elt, "shortMonths"))
        self.longDays = eltText(firstChildElt(elt, "longDays"))
        self.shortDays = eltText(firstChildElt(elt, "shortDays"))

def loadLocaleMap(doc, language_map, country_map):
    result = {}

    locale_list_elt = firstChildElt(doc.documentElement, "localeList")
    locale_elt = firstChildElt(locale_list_elt, "locale")
    while locale_elt:
        locale = Locale(locale_elt)
        language_id = languageNameToId(locale.language, language_map)
        country_id = countryNameToId(locale.country, country_map)
        result[(language_id, country_id)] = locale

        locale_elt = nextSiblingElt(locale_elt, "locale")

    return result

def compareLocaleKeys(key1, key2):
    if key1 == key2:
        return 0

    if key1[0] == key2[0]:
        l1 = compareLocaleKeys.locale_map[key1]
        l2 = compareLocaleKeys.locale_map[key2]

        if l1.language in compareLocaleKeys.default_map:
            default = compareLocaleKeys.default_map[l1.language]
            if l1.country == default:
                return -1
            if l2.country == default:
                return 1
    else:
        return key1[0] - key2[0]

    return key1[1] - key2[1]


def languageCount(language_id, locale_map):
    result = 0
    for key in locale_map.keys():
        if key[0] == language_id:
            result += 1
    return result

class StringData:
    def __init__(self):
        self.data = []
    def findIndex(self, s):
        s = s.encode("utf-8")
        result = 0
        for d in self.data:
            if d == s:
                return result
            result += len(d) + 1
        self.data.append(s)
        return result

def printEscapedString(s):
    line = ""
    need_escape = False
    for c in s:
        if ord(c) < 128 and (not need_escape or ord(c.lower()) < ord('a') or ord(c.lower()) > ord('f')):
            line += c
            need_escape = False
        else:
            line += "\\x%02x" % (ord(c))
            need_escape = True
        if len(line) > 80:
            print "\"" + line + "\""
            line = ""
    line += "\\0"
    print "\"" + line + "\""


doc = xml.dom.minidom.parse("locale.xml")
language_map = loadLanguageMap(doc)
country_map = loadCountryMap(doc)
default_map = loadDefaultMap(doc)
locale_map = loadLocaleMap(doc, language_map, country_map)
dupes = findDupes(language_map, country_map)

# Language enum
print "enum Language {"
language = ""
for key in language_map.keys():
    language = fixedLanguageName(language_map[key][0], dupes)
    print "    " + language + " = " + str(key) + ","
print "    LastLanguage = " + language
print "};"

print

# Country enum
print "enum Country {"
country = ""
for key in country_map.keys():
    country = fixedCountryName(country_map[key][0], dupes)
    print "    " + country + " = " + str(key) + ","
print "    LastCountry = " + country
print "};"

print

# Locale index
print "static const uint locale_index[] = {"
print "     0, // unused"
index = 0
for key in language_map.keys():
    i = 0
    count = languageCount(key, locale_map)
    if count > 0:
        i = index
        index += count
    print "%6d, // %s" % (i, language_map[key][0])
print "     0 // trailing 0"
print "};"

print

date_format_data = StringData()
time_format_data = StringData()
months_data = StringData()
days_data = StringData()

# Locale data
print "static const QLocalePrivate locale_data[] = {"
print "//      lang   terr    dec  group   list  prcnt   zero  minus    exp sDtFmt lDtFmt sTmFmt lTmFmt sMonth lMonth  sDays  lDays"

locale_keys = locale_map.keys()
compareLocaleKeys.default_map = default_map
compareLocaleKeys.locale_map = locale_map
locale_keys.sort(compareLocaleKeys)

for key in locale_keys:
    l = locale_map[key]

    print "    { %6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d }, // %s/%s" \
                % (key[0], key[1],
                    l.decimal,
                    l.group,
                    l.listDelim,
                    l.percent,
                    l.zero,
                    l.minus,
                    l.exp,
                    date_format_data.findIndex(l.shortDateFormat),
                    date_format_data.findIndex(l.longDateFormat),
                    time_format_data.findIndex(l.shortTimeFormat),
                    time_format_data.findIndex(l.longTimeFormat),
                    months_data.findIndex(l.shortMonths),
                    months_data.findIndex(l.longMonths),
                    days_data.findIndex(l.shortDays),
                    days_data.findIndex(l.longDays),
                    l.language,
                    l.country)
print "    {      0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 }  // trailing 0s"
print "};"

print

# Date format data
print "static const char date_format_data[] ="
for d in date_format_data.data:
    printEscapedString(d)
print ";"

print

# Time format data
print "static const char time_format_data[] ="
for d in time_format_data.data:
    printEscapedString(d)
print ";"

print

# Months data
print "static const char months_data[] ="
for d in months_data.data:
    printEscapedString(d)
print ";"

print

# Days data
print "static const char days_data[] ="
for d in days_data.data:
    printEscapedString(d)
print ";"

print

# Language name list
print "static const char language_name_list[] ="
print "\"Default\\0\""
for key in language_map.keys():
    print "\"" + language_map[key][0] + "\\0\""
print ";"

print

# Language name index
print "static const uint language_name_index[] = {"
print "     0, // Unused"
index = 8
for key in language_map.keys():
    language = language_map[key][0]
    print "%6d, // %s" % (index, language)
    index += len(language) + 1
print "};"

print

# Country name list
print "static const char country_name_list[] ="
print "\"Default\\0\""
for key in country_map.keys():
    if key == 0:
        continue
    print "\"" + country_map[key][0] + "\\0\""
print ";"

print

# Country name index
print "static const uint country_name_index[] = {"
print "     0, // AnyCountry"
index = 8
for key in country_map.keys():
    if key == 0:
        continue
    country = country_map[key][0]
    print "%6d, // %s" % (index, country)
    index += len(country) + 1
print "};"

print

# Language code list
print "static const unsigned char language_code_list[] ="
print "\"  \\0\" // Unused"
for key in language_map.keys():
    code = language_map[key][1]
    if len(code) == 2:
        code += r"\0"
    print "\"%2s\" // %s" % (code, language_map[key][0])
print ";"

print

# Country code list
print "static const unsigned char country_code_list[] ="
for key in country_map.keys():
    print "\"%2s\" // %s" % (country_map[key][1], country_map[key][0])
print ";"


