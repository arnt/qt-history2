var RSSLINKSVISIBLE = "rsslinksvisible";
var LATESTRSSITEMTIMESTAMP = "latestrssitemtimestamp";
var RSSLINKSSELECTOR = "#rssbox ul ul";
var TITLELINKSSELECTOR = "#rssbox>ul>li>a";
var LOGOIMAGESELECTOR = "img[@src*=trolltech-logo]";

var gRssBoxTitleLink = null;
var gLatestRssItemTimestamp = 0;

function init()
{
	gRssBoxTitleLink = $(TITLELINKSSELECTOR);
	$(window).resize(function() {recalculateRssBoxPosition();});
	$("#rssbox>ul>li>ul>li>a").click(function() {writeLatestRssItemTimestampIntoCookie();});
	$("#rssbox").css({ display: "block", position: "absolute" });
	gRssBoxTitleLink.removeAttr("href").css("cursor", "pointer").click(function() {toggleRssLinksVisibility();});
	
	setRssLinksVisible(readCookie(RSSLINKSVISIBLE) == 'yes');
	
	recalculateRssBoxPosition();
	window.setInterval("recalculateRssBoxPosition()", 3000);
	
	gLatestRssItemTimestamp = readLatestRssItemTimestampFromHTML();
	
	startPulsating();
}

function readLatestRssItemTimestampFromHTML()
{
	var timeStampString = $("#rssbox>ul>li").attr("id");
	var timeStampInteger = parseInt(timeStampString.substr(1)); // We assume that the id begins with a 'u'
	return timeStampInteger;
}

function writeLatestRssItemTimestampIntoCookie()
{
	if (gLatestRssItemTimestamp > readCookie(LATESTRSSITEMTIMESTAMP))
		createCookie(LATESTRSSITEMTIMESTAMP, gLatestRssItemTimestamp, 1000);
}

function titleShouldPulsate()
{
	return (readCookie(LATESTRSSITEMTIMESTAMP) < gLatestRssItemTimestamp);
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
	writeLatestRssItemTimestampIntoCookie();
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

	createCookie(RSSLINKSVISIBLE, visible?"yes":"no", 1000);
	gRssBoxTitleLink.css("background-image", "url(images/triangle" + (visible?"Horizontal":"Vertical") + ".png)");
}

// The following cookie handling functions are from:
// http://www.quirksmode.org/js/cookies.html#script
function createCookie(name,value,days){
	if (days) {
		var date = new Date();
		date.setTime(date.getTime()+(days*24*60*60*1000));
		var expires = "; expires="+date.toGMTString();
	}
	else var expires = "";
	document.cookie = name+"="+value+expires+"; path=/";
}

function readCookie(name) {
	var nameEQ = name + "=";
	var ca = document.cookie.split(';');
	for(var i=0;i < ca.length;i++) {
		var c = ca[i];
		while (c.charAt(0)==' ') c = c.substring(1,c.length);
		if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
	}
	return null;
}

function eraseCookie(name) {
	createCookie(name,"",-1);
}
