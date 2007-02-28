function init()
{
	$("#rssbox").css("display", "block");
	$("#rssbox>ul>li>a").removeAttr("href").css("cursor", "pointer").click(function() {toggleRssLinksVisibility();});
	setRssLinksVisible(getCookie("rsslinksvisible") == "true");
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