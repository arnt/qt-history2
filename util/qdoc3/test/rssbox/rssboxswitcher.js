function init()
{
	$(window).resize(function() {recalculateRssBoxPosition();});
	$("#rssbox").css({ display: "block", position: "absolute" });
	recalculateRssBoxPosition();
	$(TITLELINKSSELECTOR).removeAttr("href").css("cursor", "pointer").click(function() {toggleRssLinksVisibility();});
	setRssLinksVisible(getCookie(RSSLINKSVISIBLE) == "true");
	
	fadeToTransparent();
}

function rssLinksHaveBeenSeenOnce()
{
	return (getCookie(RSSLINKSHAVEBEENVISIBLEONCE) == "true");
}

function fadeToTransparent()
{
	if (!rssLinksHaveBeenSeenOnce())
		$(TITLELINKSSELECTOR).fadeTo(800, 0.3, function() {fadeToOpaque();});
}

function fadeToOpaque()
{
	$(TITLELINKSSELECTOR).fadeTo(800, 1, function() {fadeToTransparent();});
}

function recalculateRssBoxPosition()
{
	var logoWidth = $(LOGOIMAGESELECTOR).width();
	var bodyMarginTop = cleanNumber($("body").css("margin-top"));
	var bodyMarginLeft = cleanNumber($("body").css("margin-left"));
	var logoHeight = $(LOGOIMAGESELECTOR).height();
	var headerTableWidth = $("table:first").width();
	$("#rssbox").css({ left: headerTableWidth - logoWidth + bodyMarginLeft, top: logoHeight + bodyMarginTop}).width(logoWidth);
}

function cleanNumber(dirtyNumber)
{
	while (dirtyNumber.search(/[^0-9]/) >= 0)
		dirtyNumber = dirtyNumber.replace(/[^0-9]/, "");
	return dirtyNumber * 1;
}

function toggleRssLinksVisibility()
{
	setRssLinksVisible(!getIsRssLinksVisible());
	setCookie(RSSLINKSHAVEBEENVISIBLEONCE, "true");
}

function getIsRssLinksVisible()
{
	return $(RSSLINKSSELECTOR).css("display") != "none";
}

function setRssLinksVisible(visible)
{
	var linksElement = $(RSSLINKSSELECTOR);

	if (visible)
		$(linksElement).show();
	else
		$(linksElement).hide();

	setCookie(RSSLINKSVISIBLE, visible?"true":"false");
	$(TITLELINKSSELECTOR).css("background-image", "url(images/triangle" + (visible?"Horizontal":"Vertical") + ".png)");
}

function getCookie(c_name)
{
	if (document.cookie.length > 0)
	{
		c_start = document.cookie.indexOf(c_name + "=");
		if (c_start != -1)
		{ 
			c_start = c_start + c_name.length + 1;
			c_end = document.cookie.indexOf(";", c_start);
			if (c_end == -1)
				c_end = document.cookie.length;
			return unescape(document.cookie.substring(c_start, c_end));
		} 
	}
	return "";
}

function setCookie(c_name, value)
{
	document.cookie = c_name + "=" + escape(value);
}

var RSSLINKSVISIBLE = "rsslinksvisible";
var RSSLINKSHAVEBEENVISIBLEONCE = "rsslinkshavebeenvisibleonce";
var RSSLINKSSELECTOR = "#rssbox ul ul";
var TITLELINKSSELECTOR = "#rssbox>ul>li>a";
var LOGOIMAGESELECTOR = "img[@src*=trolltech-logo]";
