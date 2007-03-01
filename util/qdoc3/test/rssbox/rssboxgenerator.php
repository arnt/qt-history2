<?
	require('simplepie.inc');

	function renderRssFeedItem($item)
	{
		echo
			"<a href=\"".$item->get_permalink()."\">".
			$item->get_title().
			"</a>";
	}

	function renderRssFeed($feed)
	{
		echo
			"\t\t<a href=\"".$feed->get_feed_link()."\">".
			$feed->get_feed_title().
			"</a>\n"; 

		echo "\t\t<ul>\n";

		$itemsCount = $feed->get_item_quantity(5);
		for ($i = 0; $i < $itemsCount; $i++)
		{
			echo "\t\t\t<li>";
			renderRssFeedItem($feed->get_item($i));
			echo "</li>\n";
		}

		echo "\t\t</ul>\n";
	}
	
	function renderRssFeedList($list)
	{
		echo "<ul>\n";

		foreach ($list as $url)
		{
			$feed = new SimplePie($url);
			$feed->enable_caching(false);
			$feed->init();
			$feed->handle_content_type();
		
			if (!$feed->data)
				echo "\t\tFeed \"$feedURL\": <em>".$feed->error."</em>\n";
			else
			{
				echo "\t<li id=\"u".getTimestampOfLatestItemInFeed($feed)."\">\n"; // adding a 'u' infront of the timestam. See http://www.w3.org/TR/html4/types.html#h-6.2
				renderRssFeed($feed);
				echo "\t</li>\n";
			}
		}

		echo "</ul>\n";
	}
	
	function getTimestampOfLatestItemInFeed($feed)
	{
		$latestItem = $feed->get_item(0);
		$date = $latestItem->get_date("U");
		return $date;
	}
	
	$rssFeedURLs = array
	(
//		'http://rss.news.yahoo.com/rss/topstories',
//		'http://digg.com/rss/index.xml',
		'http://labs.trolltech.com/blogs/feed'
	);
	
	echo "<div id=\"rssbox\">\n";
	renderRssFeedList($rssFeedURLs);
	echo "</div>\n";
?>