function init()
{
	$(window).resize(function() {recalculateRssBoxPosition();});
	$("#rssbox").css({ display: "block", position: "absolute" });
	recalculateRssBoxPosition();
	$("#rssbox>ul>li>a").removeAttr("href").css("cursor", "pointer").click(function() {toggleRssLinksVisibility();});
	setRssLinksVisible(getCookie("rsslinksvisible") == "true");
}

function recalculateRssBoxPosition()
{
	var logoWidth = $("img[@src*=trolltech-logo]").width();
	var bodyMarginTop = cleanNumber($("body").css("margin-top"));
	var bodyMarginLeft = cleanNumber($("body").css("margin-left"));
	var logoHeight = $("img[@src*=trolltech-logo]").height();
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
}

function getIsRssLinksVisible()
{
	return $("#rssbox ul ul").css("display") != "none";
}

function setRssLinksVisible(visible)
{
	var linksElement = $("#rssbox ul ul");

	if (visible)
		$(linksElement).show();
	else
		$(linksElement).hide();

	setCookie("rsslinksvisible", visible?"true":"false");
	$("#rssbox>ul>li>a").css("background-image", "url(images/triangle" + (visible?"Horizontal":"Vertical") + ".png)");
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