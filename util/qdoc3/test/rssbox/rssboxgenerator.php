<?
	require('simplepie.inc');

	function renderRssFeedItem($item)
	{
		echo
			"<a href=\"".$item->get_permalink()."\">".
			$item->get_title().
			"</a>";
	}

	function renderRssFeed($feedURL)
	{
		$feed = new SimplePie($feedURL);
		$feed->enable_caching(false);
		$feed->init();
		$feed->handle_content_type();
	
		if (!$feed->data)
			echo "\t\tFeed \"$feedURL\": <em>".$feed->error."</em>\n";
		else
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
	}
	
	function renderRssFeedList($list)
	{
		echo "<ul>\n";

		foreach ($list as $url)
		{
			echo "\t<li>\n";
			renderRssFeed($url);
			echo "\t</li>\n";
		}

		echo "</ul>\n";
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