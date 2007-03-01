function init()
{
	gRssBoxTitleLink = $(TITLELINKSSELECTOR);
	$(window).resize(function() {recalculateRssBoxPosition();});
	$("#rssbox").css({ display: "block", position: "absolute" });
	gRssBoxTitleLink.removeAttr("href").css("cursor", "pointer").click(function() {toggleRssLinksVisibility();});
	setRssLinksVisible(getCookie(RSSLINKSVISIBLE) == "true");
	
	recalculateRssBoxPosition();
	window.setInterval("recalculateRssBoxPosition()", 3000);
	
	startPulsating();
}

function rssLinksHaveBeenSeenOnce()
{
	return (getCookie(RSSLINKSHAVEBEENVISIBLEONCE) == "true");
}

function startPulsating()
{
	if (!rssLinksHaveBeenSeenOnce())
	{
		var element = gRssBoxTitleLink;
		element.originalBackgroundColor = $(element).css("background-color");
		element.color1 = [175, 211, 77]; // Hack: This is actually originalBackgroundColor but reading rgb values from CSS can be an adventure
		element.color2 = [element.color1[0] + 30, element.color1[1] + 30, element.color1[2] + 30];
		
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
	if (rssLinksHaveBeenSeenOnce())
	{
		stopPulsating();
		return;
	}
	var element = gRssBoxTitleLink;
	var color = element.fadingStepColors[element.fadingCurrentIndex];
	$(element).css("background-color", "rgb(" + color[0] + ", " + color[1] + ", " + color[2] + ")");
	element.fadingCurrentIndex = (element.fadingCurrentIndex + 1) % (element.fadingStepColors.length);
	var huihui = 1;
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
var RSSLINKSHAVEBEENVISIBLEONCE = "rsslinkshavebeenvisibleonce";
var RSSLINKSSELECTOR = "#rssbox ul ul";
var TITLELINKSSELECTOR = "#rssbox>ul>li>a";
var LOGOIMAGESELECTOR = "img[@src*=trolltech-logo]";

var gRssBoxTitleLink = null;

//recalculateRssBoxPosition();