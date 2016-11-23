#include "catch.hpp"
#include "test-helpers.h"

#include <cache.h>
#include <itemlist_formaction.h>
#include <keymap.h>
#include <itemlist.h>

#include <unistd.h>

using namespace newsbeuter;


TEST_CASE("OP_OPEN properly displays article elements", "[itemlist_formaction]") {
	controller c;
	newsbeuter::view v(&c);
	TestHelpers::TempFile pagerfile;

	std::string test_url = "http://test_url";
	std::string test_title = "Article Title";
	std::string test_author = "Article Author";
	std::string test_description = "Article Description";
	time_t test_pubDate = 42;
	char test_pubDate_str[128];
	strftime(test_pubDate_str, sizeof(test_pubDate_str), "%a, %d %b %Y %H:%M:%S %z", localtime(&test_pubDate));

	std::string pager_prefix_title = "Title: ";
	std::string pager_prefix_author = "Author: ";
	std::string pager_prefix_date = "Date: ";
	std::string pager_prefix_link = "Link: ";

	std::string line;
	configcontainer cfg;

	cache rsscache(":memory:", &cfg);
	cfg.set_configvalue("pager", "cat %f > " + pagerfile.getPath());

	std::shared_ptr<rss_feed> feed = std::make_shared<rss_feed>(&rsscache);

	std::shared_ptr<rss_item> item = std::make_shared<rss_item>(&rsscache);
	item->set_link(test_url);
	item->set_title(test_title);
	item->set_author(test_author);
	item->set_description(test_description);
	item->set_pubDate(test_pubDate);
	item->set_unread(true);
	feed->add_item(item);

	v.set_config_container(&cfg);
	c.set_view(&v);

	itemlist_formaction itemlist(&v, itemlist_str);
	itemlist.set_feed(feed);

	REQUIRE_NOTHROW(itemlist.process_op(OP_OPEN));

	std::ifstream pagerFileStream (pagerfile.getPath());
	REQUIRE(std::getline (pagerFileStream,line));
	REQUIRE(line == pager_prefix_title + test_title);

	REQUIRE(std::getline (pagerFileStream,line));
	REQUIRE(line == pager_prefix_author + test_author);

	REQUIRE(std::getline (pagerFileStream,line));
	REQUIRE(line == pager_prefix_date + test_pubDate_str);

	REQUIRE(std::getline (pagerFileStream,line));
	REQUIRE(line == pager_prefix_link + test_url);

	REQUIRE(std::getline (pagerFileStream,line));
	REQUIRE(line == " ");

	REQUIRE(std::getline (pagerFileStream,line));
	REQUIRE(line == test_description);

	REQUIRE(std::getline (pagerFileStream,line));
	REQUIRE(line == "");
}

TEST_CASE("OP_DELETE", "[itemlist_formaction]") {
	//REQUIRE_NOTHROW(itemlist.process_op(OP_DELETE));
	//REQUIRE(feed->total_item_count() == itemCount -1);
	//Crash, to investigate
}

TEST_CASE("OP_PURGE_DELETED works properly", "[itemlist_formaction]") {
	//Does not do much for now, 
	//Trigger deletion before in order to test properly...
	controller c;
	newsbeuter::view v(&c);
	configcontainer cfg;
	cache rsscache(":memory:", &cfg);
	std::shared_ptr<rss_feed> feed = std::make_shared<rss_feed>(&rsscache);

	v.set_config_container(&cfg);
	c.set_view(&v);

	itemlist_formaction itemlist(&v, itemlist_str);
	itemlist.set_feed(feed);
	REQUIRE_NOTHROW(itemlist.process_op(OP_PURGE_DELETED));
}

TEST_CASE("OP_OPENBROWSER_AND_MARK passes the proper url to the browser and marks read", "[itemlist_formaction]") {
	controller c;
	newsbeuter::view v(&c);
	TestHelpers::TempFile browserfile;

	std::string test_url = "http://test_url";
	std::string line;

	configcontainer cfg;
	cfg.set_configvalue("browser", "echo %u >> " + browserfile.getPath());

	cache rsscache(":memory:", &cfg);

	std::shared_ptr<rss_feed> feed = std::make_shared<rss_feed>(&rsscache);
	std::shared_ptr<rss_item> item = std::make_shared<rss_item>(&rsscache);
	item->set_link(test_url);
	item->set_unread(true);
	feed->add_item(item);

	v.set_config_container(&cfg);
	c.set_view(&v);

	itemlist_formaction itemlist(&v, itemlist_str);
	itemlist.set_feed(feed);
	itemlist.process_op(OP_OPENBROWSER_AND_MARK);
	std::ifstream browserFileStream (browserfile.getPath());

	REQUIRE(std::getline (browserFileStream,line));
	REQUIRE(line == test_url);

	REQUIRE(feed->unread_item_count() == 0);
}

TEST_CASE("OP_OPENINBROWSER passes the proper url to the browser", "[itemlist_formaction]") {
	controller c;
	newsbeuter::view v(&c);
	TestHelpers::TempFile browserfile;
	std::string test_url = "http://test_url";
	std::string line;

	configcontainer cfg;
	cfg.set_configvalue("browser", "echo %u >> " + browserfile.getPath());

	cache rsscache(":memory:", &cfg);

	std::shared_ptr<rss_feed> feed = std::make_shared<rss_feed>(&rsscache);
	std::shared_ptr<rss_item> item = std::make_shared<rss_item>(&rsscache);
	item->set_link(test_url);
	feed->add_item(item);

	v.set_config_container(&cfg);
	c.set_view(&v);

	itemlist_formaction itemlist(&v, itemlist_str);
	itemlist.set_feed(feed);
	itemlist.process_op(OP_OPENINBROWSER);
	std::ifstream browserFileStream (browserfile.getPath());

	REQUIRE(std::getline (browserFileStream,line));
	REQUIRE(line == test_url);
}

TEST_CASE("OP_OPENALLUNREADINBROWSER passes the proper url list to the browser", "[itemlist_formaction]"){
	controller c;
	newsbeuter::view v(&c);
	TestHelpers::TempFile browserfile;
	std::unordered_set<std::string> url_set;
	std::string test_url = "http://test_url";
	std::string line;
	int itemCount = 6;

	configcontainer cfg;
	cfg.set_configvalue("browser", "echo %u >> " + browserfile.getPath());

	cache rsscache(":memory:", &cfg);

	std::shared_ptr<rss_feed> feed = std::make_shared<rss_feed>(&rsscache);

	for (int i = 0; i < itemCount; i++) {
		std::shared_ptr<rss_item> item = std::make_shared<rss_item>(&rsscache);
		item->set_link(test_url + std::to_string(i));
		url_set.insert(test_url + std::to_string(i));
		item->set_unread(true);
		feed->add_item(item);
	}

	v.set_config_container(&cfg);
	c.set_view(&v);

	itemlist_formaction itemlist(&v, itemlist_str);
	itemlist.set_feed(feed);

	SECTION("unread >= max-browser-tabs"){
		int maxItemsToOpen = 4;
		int openedItemsCount = 0;
		cfg.set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));

		itemlist.process_op(OP_OPENALLUNREADINBROWSER);

		std::ifstream browserFileStream (browserfile.getPath());
		openedItemsCount = 0;
		if (browserFileStream.is_open()) {
			while ( std::getline (browserFileStream,line)) {
				INFO("Each URL should be present exactly once. Erase urls after first match to fail if an item opens twice.")
				REQUIRE(url_set.count(line) == 1);
				url_set.erase(url_set.find(line));
				openedItemsCount += 1;
			}
		}
		REQUIRE(openedItemsCount == maxItemsToOpen);
	}

	SECTION("unread < max-browser-tabs"){
		int maxItemsToOpen = 9;
		int openedItemsCount = 0;
		cfg.set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));

		itemlist.process_op(OP_OPENALLUNREADINBROWSER);

		std::ifstream browserFileStream (browserfile.getPath());
		if (browserFileStream.is_open()) {
			while ( std::getline (browserFileStream,line)) {
				INFO("Each URL should be present exactly once. Erase urls after first match to fail if an item opens twice.")
				REQUIRE(url_set.count(line) == 1);
				url_set.erase(url_set.find(line));
				openedItemsCount += 1;
			}
		}
		REQUIRE(openedItemsCount == itemCount);
	}
}

TEST_CASE("OP_OPENALLUNREADINBROWSER_AND_MARK passes the proper url list to the browser and marks them read", "[itemlist_formaction]"){
	controller c;
	newsbeuter::view v(&c);
	TestHelpers::TempFile browserfile;
	std::unordered_set<std::string> url_set;
	std::string test_url = "http://test_url";
	std::string line;
	int itemCount = 6;

	configcontainer cfg;
	cfg.set_configvalue("browser", "echo %u >> " + browserfile.getPath());

	cache rsscache(":memory:", &cfg);

	std::shared_ptr<rss_feed> feed = std::make_shared<rss_feed>(&rsscache);

	for (int i = 0; i < itemCount; i++) {
		std::shared_ptr<rss_item> item = std::make_shared<rss_item>(&rsscache);
		item->set_link(test_url + std::to_string(i));
		url_set.insert(test_url + std::to_string(i));
		item->set_unread(true);
		feed->add_item(item);
	}

	v.set_config_container(&cfg);
	c.set_view(&v);

	itemlist_formaction itemlist(&v, itemlist_str);
	itemlist.set_feed(feed);

	SECTION("unread >= max-browser-tabs"){
		int maxItemsToOpen = 4;
		int openedItemsCount = 0;
		cfg.set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));

		itemlist.process_op(OP_OPENALLUNREADINBROWSER_AND_MARK);

		std::ifstream browserFileStream (browserfile.getPath());
		if (browserFileStream.is_open()) {
			while ( std::getline (browserFileStream,line)) {
				INFO("Each URL should be present exactly once. Erase urls after first match to fail if an item opens twice.")
				REQUIRE(url_set.count(line) == 1);
				url_set.erase(url_set.find(line));
				openedItemsCount += 1;
			}
		}
		REQUIRE(openedItemsCount == maxItemsToOpen);
		REQUIRE(feed->unread_item_count() == itemCount - maxItemsToOpen);
	}

	SECTION("unread < max-browser-tabs"){
		int maxItemsToOpen = 9;
		int openedItemsCount = 0;
		cfg.set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));

		itemlist.process_op(OP_OPENALLUNREADINBROWSER_AND_MARK);

		std::ifstream browserFileStream (browserfile.getPath());
		if (browserFileStream.is_open()) {
			while ( std::getline (browserFileStream,line)) {
				INFO("Each URL should be present exactly once. Erase urls after first match to fail if an item opens twice.")
				REQUIRE(url_set.count(line) == 1);
				url_set.erase(url_set.find(line));
				openedItemsCount += 1;
			}
		}
		REQUIRE(openedItemsCount == itemCount);
		REQUIRE(feed->unread_item_count() == 0);
	}
}

TEST_CASE("OP_TOGGLEITEMREAD properly switches the read status", "[itemlist_formaction]") {
#if 0
	controller c;
	newsbeuter::view v(&c);
	configcontainer cfg;
	cache rsscache(":memory:", &cfg);

	v.set_config_container(&cfg);
	c.set_view(&v);

	std::shared_ptr<rss_feed> feed = std::make_shared<rss_feed>(&rsscache);
	std::shared_ptr<rss_item> item = std::make_shared<rss_item>(&rsscache);

	SECTION("Toggle item from read to unread") {
		item->set_unread(false);
		feed->add_item(item);
		itemlist_formaction itemlist(&v, itemlist_str);
		itemlist.set_feed(feed);

		REQUIRE_NOTHROW(itemlist.process_op(OP_TOGGLEITEMREAD));
		REQUIRE(feed->unread_item_count() == 1);
	}
	SECTION("Toggle item from unread to read") {
		item->set_unread(true);
		feed->add_item(item);
		itemlist_formaction itemlist(&v, itemlist_str);
		itemlist.set_feed(feed);

		REQUIRE_NOTHROW(itemlist.process_op(OP_TOGGLEITEMREAD));
		REQUIRE(feed->unread_item_count() == 0);
	}
	SECTION("toggleitemread-jumps-to-next-unread") {
		//NOTIMPL
	}
#endif
}

TEST_CASE("OP_SHOWURLS properly shows the article's properties", "[itemlist_formaction]") {
	controller c;
	newsbeuter::view v(&c);
	configcontainer cfg;
	cache rsscache(":memory:", &cfg);
	TestHelpers::TempFile urlFile;
	std::string line;

	std::string test_url = "http://test_url";
	std::string test_title = "Article Title";
	std::string test_author = "Article Author";
	std::string test_description = "Article Description";
	time_t test_pubDate = 42;
	char test_pubDate_str[128];
	strftime(test_pubDate_str, sizeof(test_pubDate_str), "%a, %d %b %Y %H:%M:%S %z", localtime(&test_pubDate));

	std::string pager_prefix_title = "Title: ";
	std::string pager_prefix_author = "Author: ";
	std::string pager_prefix_date = "Date: ";
	std::string pager_prefix_link = "Link: ";

	v.set_config_container(&cfg);
	c.set_view(&v);


	std::shared_ptr<rss_feed> feed = std::make_shared<rss_feed>(&rsscache);
	std::shared_ptr<rss_item> item = std::make_shared<rss_item>(&rsscache);

	item->set_link(test_url);
	item->set_title(test_title);
	item->set_author(test_author);
	item->set_description(test_description);
	item->set_pubDate(test_pubDate);
	itemlist_formaction itemlist(&v, itemlist_str);


	SECTION("with external-url-viewer"){
		feed->add_item(item);
		itemlist.set_feed(feed);
		cfg.set_configvalue("external-url-viewer", "tee > " + urlFile.getPath());
	
		REQUIRE_NOTHROW(itemlist.process_op(OP_SHOWURLS));

		std::ifstream urlFileStream (urlFile.getPath());
		REQUIRE(std::getline (urlFileStream,line));
		REQUIRE(line == pager_prefix_title + test_title);

		REQUIRE(std::getline (urlFileStream,line));
		REQUIRE(line == pager_prefix_author + test_author);

		REQUIRE(std::getline (urlFileStream,line));
		REQUIRE(line == pager_prefix_date + test_pubDate_str);

		REQUIRE(std::getline (urlFileStream,line));
		REQUIRE(line == pager_prefix_link + test_url);

		REQUIRE(std::getline (urlFileStream,line));
		REQUIRE(line == " ");

		REQUIRE(std::getline (urlFileStream,line));
		REQUIRE(line == test_description);

		REQUIRE(std::getline (urlFileStream,line));
		REQUIRE(line == "");
	}

	SECTION("default behaviour"){
		feed->add_item(item);
		itemlist.set_feed(feed);
		REQUIRE_NOTHROW(itemlist.process_op(OP_SHOWURLS));
	}

	SECTION("no feed in formaction"){
		REQUIRE_NOTHROW(itemlist.process_op(OP_SHOWURLS));
	}
	
}

TEST_CASE("OP_BOOKMARK works properly", "[itemlist_formaction]") {
	controller c;
	newsbeuter::view v(&c);
	configcontainer * cfg = c.get_cfg();
	cache rsscache(":memory:", cfg);
	TestHelpers::TempFile bookmarkFile;
	std::string line;
	std::vector<std::string> bookmark_args;

	std::string test_url = "http://test_url";
	std::string test_title = "Article Title";
	std::string feed_title = "Feed Title";
	std::string separator = " ";
	std::string extra_arg = "extra arg";

	v.set_config_container(cfg);
	c.set_view(&v);

	std::shared_ptr<rss_feed> feed = std::make_shared<rss_feed>(&rsscache);
	feed->set_title(feed_title);

	std::shared_ptr<rss_item> item = std::make_shared<rss_item>(&rsscache);
	item->set_link(test_url);
	item->set_title(test_title);

	itemlist_formaction itemlist(&v, itemlist_str);

	feed->add_item(item);
	itemlist.set_feed(feed);
	
	cfg->set_configvalue("bookmark-cmd", "echo > "+ bookmarkFile.getPath());

	bookmark_args.push_back(extra_arg);
	REQUIRE_NOTHROW(itemlist.process_op(OP_BOOKMARK, true,  &bookmark_args));

	std::ifstream browserFileStream (bookmarkFile.getPath());

	REQUIRE(std::getline (browserFileStream,line));
	REQUIRE(line == test_url + separator + test_title + separator + extra_arg + separator + feed_title);

}

TEST_CASE("process_op(OP_EDITFLAGS)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_SAVE)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_HELP)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_RELOAD)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_QUIT)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_HARDQUIT)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_NEXTUNREAD)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_PREVUNREAD)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_NEXT)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_PREV)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_RANDOMUNREAD)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_NEXTUNREADFEED)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_PREVUNREADFEED)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_NEXTFEED)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_PREVFEED)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_MARKFEEDREAD)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_TOGGLESHOWREAD)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_PIPE_TO)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_SEARCH)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_EDIT_URLS)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_SELECTFILTER)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_SETFILTER)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_CLEARFILTER)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_SORT)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_REVSORT)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_INT_RESIZE)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_INT_END_SETFILTER)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_INT_EDITFLAGS_END)", "[itemlist_formaction]") {
	//NOTIMPL
}

TEST_CASE("process_op(OP_INT_START_SEARCH)", "[itemlist_formaction]") {
	//NOTIMPL
}

