<!DOCTYPE xsl:stylesheet [
     <!ENTITY endl "&#10;">
]>

<!-- This schema is used to generate the locale database in C++ from locale.xml.
     
     Usage: sabcmd generate_database.xslt locale.xml > foo
     
     Then you need to copy/paste from foo to qlocale.h (the language and country enums) 
     and qlocale.cpp (the rest). -->

<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xs="http://www.w3.org/2001/XMLSchema">

    <xsl:output method="text"/>
    
    
    <xsl:template name="generate-language-enum">
        <xsl:param name="languageList"/>
        <xsl:param name="countryList"/>
        
        <xsl:variable name="languageCount" select="count($languageList/language)"/>
        
        <xsl:text>enum Language {&endl;</xsl:text>
        <xsl:for-each select="$languageList/language">
            <xsl:variable name="languageName" select="name"/>
            <xsl:variable name="fixedLanguageName">
                <xsl:choose>
                    <xsl:when test="boolean($countryList/country[name=$languageName])">
                        <xsl:value-of select="concat($languageName, 'Language')"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="$languageName"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:variable>
        
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$fixedLanguageName"/>
            <xsl:text> = </xsl:text>
            <xsl:value-of select="id"/>
            <xsl:text>,&endl;</xsl:text>
        </xsl:for-each>
        <xsl:text>    LastLanguage = </xsl:text>
        <xsl:value-of select="$languageList/language[$languageCount]/name"/>
        <xsl:text>&endl;};&endl;</xsl:text>
    </xsl:template>
               
     
    <xsl:template name="generate-country-enum">
        <xsl:param name="languageList"/>
        <xsl:param name="countryList"/>
        
        <xsl:variable name="countryCount" select="count($countryList/country)"/>
        
        <xsl:text>enum Country {&endl;</xsl:text>
        <xsl:for-each select="$countryList/country">
            <xsl:variable name="countryName" select="name"/>
            <xsl:variable name="fixedCountryName">
                <xsl:choose>
                    <xsl:when test="boolean($languageList/language[name=$countryName])">
                        <xsl:value-of select="concat($countryName, 'Country')"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="$countryName"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:variable>
            
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$fixedCountryName"/>
            <xsl:text> = </xsl:text>
            <xsl:value-of select="id"/>
            <xsl:text>,&endl;</xsl:text>
        </xsl:for-each>
        <xsl:text>    LastCountry = </xsl:text>
        <xsl:value-of select="$countryList/country[$countryCount]/name"/>
        <xsl:text>&endl;};&endl;</xsl:text>
    </xsl:template>

    
    <xsl:template name="find-locale-index">
        <xsl:param name="localeList"/>
        <xsl:param name="languageName"/>
        <xsl:param name="index" select="0"/>
        
        <xsl:choose>
            <xsl:when test="$index=count($localeList/locale)">
                <xsl:text>0</xsl:text>
            </xsl:when>
            <xsl:when test="$localeList/locale[$index + 1]/language=$languageName">
                <xsl:value-of select="$index"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:call-template name="find-locale-index">
                    <xsl:with-param name="localeList" select="$localeList"/>
                    <xsl:with-param name="languageName" select="$languageName"/>
                    <xsl:with-param name="index" select="$index + 1"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    
    <xsl:template name="right-justify">
        <xsl:param name="value"/>
        <xsl:param name="width"/>
        
        <xsl:choose>
            <xsl:when test="string-length($value) >= $width">
                <xsl:value-of select="$value"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:call-template name="right-justify">
                    <xsl:with-param name="value" select="concat(' ', $value)"/>
                    <xsl:with-param name="width" select="$width"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    
    <xsl:template name="generate-locale-index">
        <xsl:param name="languageList"/>
        <xsl:param name="localeList"/>

        <xsl:text>static const uint locale_index[] = {&endl;</xsl:text>
        <xsl:text>     0, // unused&endl;</xsl:text>
        <xsl:for-each select="$languageList/language">
            <xsl:variable name="name"><xsl:value-of select="name"/></xsl:variable>
            <xsl:variable name="index">
                <xsl:call-template name="right-justify">
                    <xsl:with-param name="value">
                        <xsl:call-template name="find-locale-index">
                            <xsl:with-param name="localeList" select="$localeList"/>
                            <xsl:with-param name="languageName" select="$name"/>
                        </xsl:call-template>
                    </xsl:with-param>
                    <xsl:with-param name="width" select="3"/>
                </xsl:call-template>
            </xsl:variable>
            
            <xsl:text>   </xsl:text>
            <xsl:value-of select="$index"/>
            <xsl:text>, // </xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:text>&endl;</xsl:text>
        </xsl:for-each>
        <xsl:text>     0  // trailing 0&endl;</xsl:text>
        <xsl:text>};&endl;</xsl:text>
    </xsl:template>
        
    
    <xsl:template name="generate-locale-data">
        <xsl:param name="languageList"/>
        <xsl:param name="localeList"/>
        <xsl:param name="countryList"/>

        <xsl:text>static const QLocalePrivate locale_data[] = {&endl;</xsl:text>
        <xsl:text>//      lang   terr    dec  group   list  prcnt   zero  minus    exp&endl;</xsl:text>
        <xsl:for-each select="$localeList/locale">
            <xsl:variable name="languageName" select="language"/>
            <xsl:variable name="countryName" select="country"/>
        
            <xsl:text>    { </xsl:text>
            
            <xsl:call-template name="right-justify">
                <xsl:with-param name="value">
                    <xsl:value-of select="$languageList/language[name=$languageName]/id"/>
                </xsl:with-param>
                <xsl:with-param name="width" select="6"/>
            </xsl:call-template>
            
            <xsl:text>,</xsl:text>
            
            <xsl:call-template name="right-justify">
                <xsl:with-param name="value">
                    <xsl:value-of select="$countryList/country[name=$countryName]/id"/>
                </xsl:with-param>
                <xsl:with-param name="width" select="6"/>
            </xsl:call-template>
            
            <xsl:text>,</xsl:text>
            
            <xsl:call-template name="right-justify">
                <xsl:with-param name="value">
                    <xsl:value-of select="decimal"/>
                </xsl:with-param>
                <xsl:with-param name="width" select="6"/>
            </xsl:call-template>
            
            <xsl:text>,</xsl:text>
            
            <xsl:call-template name="right-justify">
                <xsl:with-param name="value">
                    <xsl:value-of select="group"/>
                </xsl:with-param>
                <xsl:with-param name="width" select="6"/>
            </xsl:call-template>
            
            <xsl:text>,</xsl:text>
            
            <xsl:call-template name="right-justify">
                <xsl:with-param name="value">
                    <xsl:value-of select="list"/>
                </xsl:with-param>
                <xsl:with-param name="width" select="6"/>
            </xsl:call-template>
            
            <xsl:text>,</xsl:text>
            
            <xsl:call-template name="right-justify">
                <xsl:with-param name="value">
                    <xsl:value-of select="percent"/>
                </xsl:with-param>
                <xsl:with-param name="width" select="6"/>
            </xsl:call-template>
            
            <xsl:text>,</xsl:text>
            
            <xsl:call-template name="right-justify">
                <xsl:with-param name="value">
                    <xsl:value-of select="zero"/>
                </xsl:with-param>
                <xsl:with-param name="width" select="6"/>
            </xsl:call-template>
            
            <xsl:text>,</xsl:text>
            
            <xsl:call-template name="right-justify">
                <xsl:with-param name="value">
                    <xsl:value-of select="minus"/>
                </xsl:with-param>
                <xsl:with-param name="width" select="6"/>
            </xsl:call-template>
            
            <xsl:text>,</xsl:text>
            
            <xsl:call-template name="right-justify">
                <xsl:with-param name="value">
                    <xsl:value-of select="exp"/>
                </xsl:with-param>
                <xsl:with-param name="width" select="6"/>
            </xsl:call-template>
            
            <xsl:text> }, // </xsl:text>
            <xsl:value-of select="concat($languageName, '/', $countryName)"/>
            <xsl:text>&endl;</xsl:text>
            
        </xsl:for-each>
        <xsl:text>    {      0,     0,     0,     0,     0,     0,     0,     0,     0 }  // trailing 0s&endl;</xsl:text>
        <xsl:text>};&endl;</xsl:text>
    </xsl:template>

    
    <xsl:template name="generate-language-name-list">
        <xsl:param name="languageList"/>
        
        <xsl:text>static const char language_name_list[] =&endl;</xsl:text>
        <xsl:text>"Default\0"&endl;</xsl:text>
        <xsl:for-each select="$languageList/language">
            <xsl:text>"</xsl:text>
            <xsl:value-of select="name"/>
            <xsl:text>\0"</xsl:text>
            <xsl:if test="position() = last()">
                <xsl:text>;</xsl:text>
            </xsl:if>
            <xsl:text>&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>
        
    
    <xsl:template name="generate-language-name-index-line">
        <xsl:param name="languageList"/>
        <xsl:param name="index" select="1"/>
        <xsl:param name="accumulator"/>
        
        <xsl:variable name="languageName" select="$languageList/language[$index]/name"/>
        
        <xsl:call-template name="right-justify">
            <xsl:with-param name="value" select="$accumulator"/>
            <xsl:with-param name="width" select="6"/>
        </xsl:call-template>
        <xsl:choose>
            <xsl:when test="$index = count($languageList/language)">
                <xsl:text> </xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>,</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text> // </xsl:text>
        <xsl:value-of select="$languageName"/>
        <xsl:text>&endl;</xsl:text>
        
        <xsl:if test="$index &lt; count($languageList/language)">
            <xsl:call-template name="generate-language-name-index-line">
                <xsl:with-param name="languageList" select="$languageList"/>
                <xsl:with-param name="index" select="$index + 1"/>
                <xsl:with-param name="accumulator" select="$accumulator + string-length($languageName) + 1"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
    
    
    <xsl:template name="generate-language-name-index">
        <xsl:param name="languageList"/>
        
        <xsl:text>static const uint language_name_index[] = {&endl;</xsl:text>
        <xsl:text>     0, // Unused&endl;</xsl:text>
        <xsl:call-template name="generate-language-name-index-line">
            <xsl:with-param name="languageList" select="languageList"/>
            <xsl:with-param name="accumulator" select="8"/>
        </xsl:call-template>
        <xsl:text>};&endl;</xsl:text>
    </xsl:template>
    
    
    <xsl:template name="generate-country-name-list">
        <xsl:param name="countryList"/>
        
        <xsl:text>static const char country_name_list[] =&endl;</xsl:text>
        <xsl:text>"Default\0"&endl;</xsl:text>
        <xsl:for-each select="$countryList/country[position() > 1]">
            <xsl:text>"</xsl:text>
            <xsl:value-of select="name"/>
            <xsl:text>\0"</xsl:text>
            <xsl:if test="position() = last()">
                <xsl:text>;</xsl:text>
            </xsl:if>
            <xsl:text>&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>
        
    
    <xsl:template name="generate-country-name-index-line">
        <xsl:param name="countryList"/>
        <xsl:param name="index" select="1"/>
        <xsl:param name="accumulator"/>
        
        <xsl:variable name="countryName" select="$countryList/country[$index]/name"/>
        
        <xsl:call-template name="right-justify">
            <xsl:with-param name="value" select="$accumulator"/>
            <xsl:with-param name="width" select="6"/>
        </xsl:call-template>
        <xsl:choose>
            <xsl:when test="$index = count($countryList/country)">
                <xsl:text> </xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>,</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text> // </xsl:text>
        <xsl:value-of select="$countryName"/>
        <xsl:text>&endl;</xsl:text>
        
        <xsl:if test="$index &lt; count($countryList/country)">
            <xsl:call-template name="generate-country-name-index-line">
                <xsl:with-param name="countryList" select="$countryList"/>
                <xsl:with-param name="index" select="$index + 1"/>
                <xsl:with-param name="accumulator" select="$accumulator + string-length($countryName) + 1"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
    
    
    <xsl:template name="generate-country-name-index">
        <xsl:param name="countryList"/>
        
        <xsl:text>static const uint country_name_index[] = {&endl;</xsl:text>
        <xsl:text>     0, // AnyCountry&endl;</xsl:text>
        <xsl:call-template name="generate-country-name-index-line">
            <xsl:with-param name="countryList" select="countryList"/>
            <xsl:with-param name="accumulator" select="8"/>
            <xsl:with-param name="index" select="2"/>
        </xsl:call-template>
        <xsl:text>};&endl;</xsl:text>
    </xsl:template>
    
    
    <xsl:template name="generate-language-code-list">
        <xsl:param name="languageList"/>
        
        <xsl:text>static const unsigned char language_code_list[] =&endl;</xsl:text>
        <xsl:text>"  " // Unused&endl;</xsl:text>
        <xsl:for-each select="$languageList/language">
            <xsl:text>"</xsl:text>
            <xsl:value-of select="code"/>
            <xsl:text>"</xsl:text>
            <xsl:choose>
                <xsl:when test="position() = last()">
                    <xsl:text>;</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text> </xsl:text>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:text>// </xsl:text>
            <xsl:value-of select="name"/>
            <xsl:text>&endl;</xsl:text>
        </xsl:for-each>        
    </xsl:template>

    
    <xsl:template name="generate-country-code-list">
        <xsl:param name="countryList"/>
        
        <xsl:text>static const unsigned char country_code_list[] =&endl;</xsl:text>
        <xsl:for-each select="$countryList/country">
            <xsl:text>"</xsl:text>
            <xsl:value-of select="code"/>
            <xsl:text>"</xsl:text>
            <xsl:choose>
                <xsl:when test="position() = last()">
                    <xsl:text>;</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text> </xsl:text>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:text>// </xsl:text>
            <xsl:value-of select="name"/>
            <xsl:text>&endl;</xsl:text>
        </xsl:for-each>        
    </xsl:template>
    
    
    <xsl:template match="localeDatabase">
        <xsl:call-template name="generate-language-enum">
            <xsl:with-param name="languageList" select="languageList"/>
            <xsl:with-param name="countryList" select="countryList"/>
        </xsl:call-template>
        
        <xsl:text>&endl;</xsl:text>
        
        <xsl:call-template name="generate-country-enum">
            <xsl:with-param name="languageList" select="languageList"/>
            <xsl:with-param name="countryList" select="countryList"/>
        </xsl:call-template>
    
        <xsl:text>&endl;</xsl:text>
    
        <xsl:call-template name="generate-locale-index">
            <xsl:with-param name="languageList" select="languageList"/>
            <xsl:with-param name="localeList" select="localeList"/>
        </xsl:call-template>
        
        <xsl:text>&endl;</xsl:text>
        
        <xsl:call-template name="generate-locale-data">
            <xsl:with-param name="languageList" select="languageList"/>
            <xsl:with-param name="countryList" select="countryList"/>
            <xsl:with-param name="localeList" select="localeList"/>
        </xsl:call-template>
    
        <xsl:text>&endl;</xsl:text>
        
        <xsl:call-template name="generate-language-name-list">
            <xsl:with-param name="languageList" select="languageList"/>
        </xsl:call-template>
        
        <xsl:text>&endl;</xsl:text>
        
        <xsl:call-template name="generate-language-name-index">
            <xsl:with-param name="languageList" select="languageList"/>
        </xsl:call-template>
    
        <xsl:text>&endl;</xsl:text>
        
        <xsl:call-template name="generate-country-name-list">
            <xsl:with-param name="countryList" select="countryList"/>
        </xsl:call-template>
        
        <xsl:text>&endl;</xsl:text>
        
        <xsl:call-template name="generate-country-name-index">
            <xsl:with-param name="countryList" select="countryList"/>
        </xsl:call-template>
        
        <xsl:text>&endl;</xsl:text>
        
        <xsl:call-template name="generate-language-code-list">
            <xsl:with-param name="languageList" select="languageList"/>
        </xsl:call-template>
        
        <xsl:text>&endl;</xsl:text>
        
        <xsl:call-template name="generate-country-code-list">
            <xsl:with-param name="countryList" select="countryList"/>
        </xsl:call-template>
        
    </xsl:template>
                
</xsl:stylesheet>
