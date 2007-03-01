function init()
{
	gRssBoxTitleLink = $(TITLELINKSSELECTOR);
	$(window).resize(function() {recalculateRssBoxPosition();});
	$("#rssbox").css({ display: "block", position: "absolute" });
	gRssBoxTitleLink.removeAttr("href").css("cursor", "pointer").click(function() {toggleRssLinksVisibility();});
	
	setRssLinksVisible(getCookie(RSSLINKSVISIBLE) == 'yes');
	
	recalculateRssBoxPosition();
	window.setInterval("recalculateRssBoxPosition()", 3000);
	
	gLatestRssItemUrl = readLatestRssItemURLFromHTML();
	
	startPulsating();
}

function readLatestRssItemURLFromHTML()
{
	return $("#rssbox>ul>li>ul>li>a").attr("href");
}

function writeLatestRssItemURLIntoCookie()
{
	setCookie(LATESTRSSITEMURL, gLatestRssItemUrl);
}

function titleShouldPulsate()
{
//	return true;
	return (getCookie(LATESTRSSITEMURL) != gLatestRssItemUrl);
}

function startPulsating()
{
	if (titleShouldPulsate())
	{
		var element = gRssBoxTitleLink;
		element.originalBackgroundColor = $(element).css("background-color");
		element.color1 = [175, 211, 77]; // Hack: This is actually originalBackgroundColor but reading rgb values from CSS can be an adventure
		element.color2 = [element.color1[0] + 35, element.color1[1] + 35, element.color1[2] + 35];
		
		setupFadingStepColors(element);
		
		element.interval = window.setInterval("handleFaderTimerTick()", 100);
	}
}

function setupFadingStepColors()
{
	var element = gRssBoxTitleLink;
	element.fadingStepsCount = 10;
	element.fadingCurrentIndex = 0;
	element.fadingStepColors = new Array(element.fadingStepsCount * 2);
	for (var i = 0; i < element.fadingStepsCount; i++)
	{
		element.fadingStepColors[i] = [0, 0, 0];
		var factor = 1 / element.fadingStepsCount * i;
		for (var channel = 0; channel < 3; channel ++)
		{
			var difference = element.color1[channel] - element.color2[channel];
			element.fadingStepColors[i][channel] = element.color1[channel] - difference * factor;
		}
		var descendingColorIndex = element.fadingStepsCount + (element.fadingStepsCount - i) - 1;
		element.fadingStepColors[descendingColorIndex] = element.fadingStepColors[i];
	}
}

function handleFaderTimerTick()
{
	if (!titleShouldPulsate())
	{
		stopPulsating();
		return;
	}
	var element = gRssBoxTitleLink;
	var color = element.fadingStepColors[element.fadingCurrentIndex];
	$(element).css("background-color", "rgb(" + parseInt(color[0]) + ", " + parseInt(color[1]) + ", " + parseInt(color[2]) + ")");
	element.fadingCurrentIndex = (element.fadingCurrentIndex + 1) % (element.fadingStepColors.length);
}

function stopPulsating()
{
	var element = gRssBoxTitleLink;
	$(element).css("background-color", element.originalBackgroundColor);
	clearInterval(element.interval);
}

function recalculateRssBoxPosition()
{
	var logoWidth = $(LOGOIMAGESELECTOR).width();
	var bodyMarginTop = parseInt($("body").css("margin-top"));
	var bodyMarginLeft = parseInt($("body").css("margin-left"));
	var logoHeight = $(LOGOIMAGESELECTOR).height();
	var headerTableWidth = $("table:first").width();
	$("#rssbox").css({ left: headerTableWidth - logoWidth + bodyMarginLeft, top: logoHeight + bodyMarginTop}).width(logoWidth);
}

function toggleRssLinksVisibility()
{
	setRssLinksVisible(!getIsRssLinksVisible());
	writeLatestRssItemURLIntoCookie();
}

function getIsRssLinksVisible()
{
	return $(RSSLINKSSELECTOR).css("display") != 'none';
}

function setRssLinksVisible(visible)
{
	var linksElement = $(RSSLINKSSELECTOR);

	if (visible)
		$(linksElement).css("display", "block");
	else
		$(linksElement).css("display", "none");

	setCookie(RSSLINKSVISIBLE, visible?"yes":"no");
	gRssBoxTitleLink.css("background-image", "url(images/triangle" + (visible?"Horizontal":"Vertical") + ".png)");
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
var LATESTRSSITEMURL = "latestrssitemurl";
var RSSLINKSSELECTOR = "#rssbox ul ul";
var TITLELINKSSELECTOR = "#rssbox>ul>li>a";
var LOGOIMAGESELECTOR = "img[@src*=trolltech-logo]";

var gRssBoxTitleLink = null;
var gLatestRssItemUrl = ""

//window.onload = recalculateRssBoxPosition;
