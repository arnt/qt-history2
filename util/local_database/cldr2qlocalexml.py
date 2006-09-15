#! /usr/bin/python

import os
import sys
import enumdata
import xpathlite
import re

findEntry = xpathlite.findEntry

def ordStr(c):
    if len(c) == 1:
        return str(ord(c))
    return "##########"

def generateLocaleInfo(path):
    (dir_name, file_name) = os.path.split(path)

    exp = re.compile(r"([a-z]+)_([A-Z]{2})\.xml")
    m = exp.match(file_name)
    if not m:
        return {}

    language_code = m.group(1)
    country_code = m.group(2)

    language_id = enumdata.languageCodeToId(language_code)
    if language_id == -1:
        sys.stderr.write("unnknown language code \"" + language_code + "\"\n")
        return {}
    language = enumdata.language_list[language_id][0]

    country_id = enumdata.countryCodeToId(country_code)
    if country_id == -1:
        sys.stderr.write("unnknown country code \"" + country_code + "\"\n")
        return {}
    country = enumdata.country_list[country_id][0]

    base = dir_name + "/" + language_code + "_" + country_code

    result = {}
    result['base'] = base

    result['language'] = language
    result['country'] = country
    result['language_id'] = language_id
    result['country_id'] = country_id
    result['decimal'] = findEntry(base, "numbers/symbols/decimal")
    result['group'] = findEntry(base, "numbers/symbols/group")
    result['list'] = findEntry(base, "numbers/symbols/list")
    result['percent'] = findEntry(base, "numbers/symbols/percentSign")
    result['zero'] = findEntry(base, "numbers/symbols/nativeZeroDigit")
    result['minus'] = findEntry(base, "numbers/symbols/minusSign")
    result['exp'] = findEntry(base, "numbers/symbols/exponential").lower()
    result['longDateFormat'] = findEntry(base, "dates/calendars/calendar[gregorian]/dateFormats/dateFormatLength[full]/dateFormat/pattern")
    result['shortDateFormat'] = findEntry(base, "dates/calendars/calendar[gregorian]/dateFormats/dateFormatLength[short]/dateFormat/pattern")
    result['longTimeFormat'] = findEntry(base, "dates/calendars/calendar[gregorian]/timeFormats/timeFormatLength[full]/timeFormat/pattern")
    result['shortTimeFormat'] = findEntry(base, "dates/calendars/calendar[gregorian]/timeFormats/timeFormatLength[short]/timeFormat/pattern")

    long_month_path = "dates/calendars/calendar[gregorian]/months/monthContext[format]/monthWidth[wide]/month"
    result['longMonths'] \
        = findEntry(base, long_month_path + "[1]") + ";" \
        + findEntry(base, long_month_path + "[2]") + ";" \
        + findEntry(base, long_month_path + "[3]") + ";" \
        + findEntry(base, long_month_path + "[4]") + ";" \
        + findEntry(base, long_month_path + "[5]") + ";" \
        + findEntry(base, long_month_path + "[6]") + ";" \
        + findEntry(base, long_month_path + "[7]") + ";" \
        + findEntry(base, long_month_path + "[8]") + ";" \
        + findEntry(base, long_month_path + "[9]") + ";" \
        + findEntry(base, long_month_path + "[10]") + ";" \
        + findEntry(base, long_month_path + "[11]") + ";" \
        + findEntry(base, long_month_path + "[12]") + ";"

    short_month_path = "dates/calendars/calendar[gregorian]/months/monthContext[format]/monthWidth[abbreviated]/month"
    result['shortMonths'] \
        = findEntry(base, short_month_path + "[1]") + ";" \
        + findEntry(base, short_month_path + "[2]") + ";" \
        + findEntry(base, short_month_path + "[3]") + ";" \
        + findEntry(base, short_month_path + "[4]") + ";" \
        + findEntry(base, short_month_path + "[5]") + ";" \
        + findEntry(base, short_month_path + "[6]") + ";" \
        + findEntry(base, short_month_path + "[7]") + ";" \
        + findEntry(base, short_month_path + "[8]") + ";" \
        + findEntry(base, short_month_path + "[9]") + ";" \
        + findEntry(base, short_month_path + "[10]") + ";" \
        + findEntry(base, short_month_path + "[11]") + ";" \
        + findEntry(base, short_month_path + "[12]") + ";"

    long_day_path = "dates/calendars/calendar[gregorian]/days/dayContext[format]/dayWidth[wide]/day"
    result['longDays'] \
        = findEntry(base, long_day_path + "[sun]") + ";" \
        + findEntry(base, long_day_path + "[mon]") + ";" \
        + findEntry(base, long_day_path + "[tue]") + ";" \
        + findEntry(base, long_day_path + "[wed]") + ";" \
        + findEntry(base, long_day_path + "[thu]") + ";" \
        + findEntry(base, long_day_path + "[fri]") + ";" \
        + findEntry(base, long_day_path + "[sat]") + ";"

    short_day_path = "dates/calendars/calendar[gregorian]/days/dayContext[format]/dayWidth[abbreviated]/day"
    result['shortDays'] \
        = findEntry(base, short_day_path + "[sun]") + ";" \
        + findEntry(base, short_day_path + "[mon]") + ";" \
        + findEntry(base, short_day_path + "[tue]") + ";" \
        + findEntry(base, short_day_path + "[wed]") + ";" \
        + findEntry(base, short_day_path + "[thu]") + ";" \
        + findEntry(base, short_day_path + "[fri]") + ";" \
        + findEntry(base, short_day_path + "[sat]") + ";"

    return result

def addEscapes(s):
    result = ''
    for c in s:
        n = ord(c)
        if n < 128:
            result += c
        else:
            result += "\\x"
            result += "%02x" % (n)
    return result

def unicodeStr(s):
    utf8 = s.encode('utf-8')
    return "<size>" + str(len(utf8)) + "</size><data>" + addEscapes(utf8) + "</data>"

def usage():
    print "Usage: cldr2qlocalexml.py <path-to-cldr-main>"
    sys.exit()

if len(sys.argv) != 2:
    usage()

cldr_dir = sys.argv[1]

if not os.path.isdir(cldr_dir):
    usage()

cldr_files = os.listdir(cldr_dir)

locale_database = {}
for file in cldr_files:
    l = generateLocaleInfo(cldr_dir + "/" + file)
    if not l:
        sys.stderr.write("skipping file \"" + file + "\"\n")
        continue

    locale_database[(l['language_id'], l['country_id'])] = l

locale_keys = locale_database.keys()
locale_keys.sort()

print "<localeDatabase>"
print "    <languageList>"
for id in enumdata.language_list:
    l = enumdata.language_list[id]
    print "        <language>"
    print "            <name>" + l[0] + "</name>"
    print "            <id>" + str(id) + "</id>"
    print "            <code>" + l[1] + "</code>"
    print "        </language>"
print "    </languageList>"

print "    <countryList>"
for id in enumdata.country_list:
    l = enumdata.country_list[id]
    print "        <country>"
    print "            <name>" + l[0] + "</name>"
    print "            <id>" + str(id) + "</id>"
    print "            <code>" + l[1] + "</code>"
    print "        </country>"
print "    </countryList>"

print \
"    <defaultCountryList>\n\
        <defaultCountry>\n\
            <language>Afrikaans</language>\n\
            <country>SouthAfrica</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Afan</language>\n\
            <country>Ethiopia</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Afar</language>\n\
            <country>Djibouti</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Arabic</language>\n\
            <country>SaudiArabia</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Chinese</language>\n\
            <country>China</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Dutch</language>\n\
            <country>Netherlands</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>English</language>\n\
            <country>UnitedStates</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>French</language>\n\
            <country>France</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>German</language>\n\
            <country>Germany</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Greek</language>\n\
            <country>Greece</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Italian</language>\n\
            <country>Italy</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Malay</language>\n\
            <country>Malaysia</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Portuguese</language>\n\
            <country>Portugal</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Russian</language>\n\
            <country>RussianFederation</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Serbian</language>\n\
            <country>SerbiaAndMontenegro</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>SerboCroatian</language>\n\
            <country>SerbiaAndMontenegro</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Somali</language>\n\
            <country>Somalia</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Spanish</language>\n\
            <country>Spain</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Swahili</language>\n\
            <country>Kenya</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Swedish</language>\n\
            <country>Sweden</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Tigrinya</language>\n\
            <country>Eritrea</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Uzbek</language>\n\
            <country>Uzbekistan</country>\n\
        </defaultCountry>\n\
        <defaultCountry>\n\
            <language>Persian</language>\n\
            <country>Iran</country>\n\
        </defaultCountry>\n\
    </defaultCountryList>"

print "    <localeList>"
print \
"        <locale>\n\
            <language>C</language>\n\
            <country>AnyCountry</country>\n\
            <decimal>46</decimal>\n\
            <group>44</group>\n\
            <list>59</list>\n\
            <percent>37</percent>\n\
            <zero>48</zero>\n\
            <minus>45</minus>\n\
            <exp>101</exp>\n\
            <longDateFormat>EEEE, d MMMM yyyy</longDateFormat>\n\
            <shortDateFormat>d MMM yyyy</shortDateFormat>\n\
            <longTimeFormat>HH:mm:ss z</longTimeFormat>\n\
            <shortTimeFormat>HH:mm:ss</shortTimeFormat>\n\
            <longMonths>January;February;March;April;May;June;July;August;September;October;November;December;</longMonths>\n\
            <shortMonths>Jan;Feb;Mar;Apr;May;Jun;Jul;Aug;Sep;Oct;Nov;Dec;</shortMonths>\n\
            <longDays>Sunday;Monday;Tuesday;Wednesday;Thursday;Friday;Saturday;</longDays>\n\
            <shortDays>Sun;Mon;Tue;Wed;Thu;Fri;Sat;</shortDays>\n\
        </locale>"

for key in locale_keys:
    l = locale_database[key]

    print "        <locale>"
#    print "            <source>"   + l['base']            + "</source>"
    print "            <language>" + l['language']        + "</language>"
    print "            <country>"  + l['country']         + "</country>"
    print "            <decimal>"  + ordStr(l['decimal']) + "</decimal>"
    print "            <group>"    + ordStr(l['group'])   + "</group>"
    print "            <list>"     + ordStr(l['list'])    + "</list>"
    print "            <percent>"  + ordStr(l['percent']) + "</percent>"
    print "            <zero>"     + ordStr(l['zero'])    + "</zero>"
    print "            <minus>"    + ordStr(l['minus'])   + "</minus>"
    print "            <exp>"      + ordStr(l['exp'])     + "</exp>"
    print "            <longDateFormat>"  + l['longDateFormat'].encode('utf-8')  + "</longDateFormat>"
    print "            <shortDateFormat>" + l['shortDateFormat'].encode('utf-8') + "</shortDateFormat>"
    print "            <longTimeFormat>"  + l['longTimeFormat'].encode('utf-8')  + "</longTimeFormat>"
    print "            <shortTimeFormat>" + l['shortTimeFormat'].encode('utf-8') + "</shortTimeFormat>"
    print "            <longMonths>"      + l['longMonths'].encode('utf-8')      + "</longMonths>"
    print "            <shortMonths>"     + l['shortMonths'].encode('utf-8')     + "</shortMonths>"
    print "            <longDays>"        + l['longDays'].encode('utf-8')        + "</longDays>"
    print "            <shortDays>"       + l['shortDays'].encode('utf-8')       + "</shortDays>"
    print "        </locale>"
print "    </localeList>"
print "</localeDatabase>"
