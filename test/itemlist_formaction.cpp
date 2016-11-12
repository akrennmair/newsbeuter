#include "catch.hpp"
#include "test-helpers.h"

#include <cache.h>
#include <itemlist_formaction.h>
#include <keymap.h>
#include <itemlist.h>

#include <unistd.h>

using namespace newsbeuter;

TEST_CASE("process_op()", "[itemlist_formaction]") {
	controller c;
	newsbeuter::view v(&c);
	TestHelpers::TempFile browserfile;
	std::unordered_set<std::string> url_set;

	std::vector<std::shared_ptr<rss_feed>> feedlist;

	std::string test_url = "http://test_url";
	std::string test_title = "Article Title";
	std::string test_author = "Article Author";
	std::string test_description = "Article Description";
	time_t test_pubDate = 42;
	std::string test_puDate_str = "Thu, 01 Jan 1970 01:00:42 +0100";

	std::string pager_prefix_title = "Title: ";
	std::string pager_prefix_author = "Author: ";
	std::string pager_prefix_date = "Date: ";
	std::string pager_prefix_link = "Link: ";

	std::string line;
	int itemCount = 6;

	configcontainer cfg;
	cfg.set_configvalue("browser", "echo %u >> " + browserfile.getPath());

	cache rsscache(":memory:", &cfg);

	std::shared_ptr<rss_feed> feed(new rss_feed(&rsscache));

	for (int i = 0; i < itemCount; i++) {
		std::shared_ptr<rss_item> item = std::make_shared<rss_item>(&rsscache);
		item->set_link(test_url + std::to_string(i));
		item->set_title(test_title);
		item->set_author(test_author);
		item->set_description(test_description);
		item->set_pubDate(test_pubDate);
		url_set.insert(test_url + std::to_string(i));
		item->set_unread(true);
		feed->add_item(item);
	}

	v.set_config_container(&cfg);
	c.set_view(&v);

	itemlist_formaction itemlist(&v, itemlist_str);
	itemlist.set_feed(feed);

	SECTION("OP_OPEN") {
		std::string articleLine;
		TestHelpers::TempFile pagerfile;
		cfg.set_configvalue("pager", "cat %f > " + pagerfile.getPath());

		REQUIRE_NOTHROW(itemlist.process_op(OP_OPEN));

		std::ifstream pagerFileStream (pagerfile.getPath());
		REQUIRE( std::getline (pagerFileStream,articleLine) );
		REQUIRE(articleLine == pager_prefix_title + test_title);

		REQUIRE( std::getline (pagerFileStream,articleLine) );
		REQUIRE(articleLine == pager_prefix_author + test_author);

		REQUIRE( std::getline (pagerFileStream,articleLine) );
		REQUIRE(articleLine == pager_prefix_date + test_puDate_str);

		REQUIRE( std::getline (pagerFileStream,articleLine) );
		REQUIRE(articleLine.find(pager_prefix_link + test_url) == 0);

		REQUIRE( std::getline (pagerFileStream,articleLine) );
		REQUIRE(articleLine == " ");

		REQUIRE( std::getline (pagerFileStream,articleLine) );
		REQUIRE(articleLine == test_description);

		REQUIRE( std::getline (pagerFileStream,articleLine) );
		REQUIRE(articleLine == "");

		pagerFileStream.close();
	}
	SECTION("OP_DELETE") {
		//REQUIRE_NOTHROW(itemlist.process_op(OP_DELETE));
		//REQUIRE(feed->total_item_count() == itemCount -1);
		//Crash, to investigate
	}
	SECTION("OP_PURGE_DELETED") {
		REQUIRE_NOTHROW(itemlist.process_op(OP_PURGE_DELETED));
		//Trigger deletion before in order to test properly...
	}
	SECTION("OP_OPENBROWSER_AND_MARK") {
		itemlist.process_op(OP_OPENBROWSER_AND_MARK);
		std::ifstream browserFileStream (browserfile.getPath());

		REQUIRE ( std::getline (browserFileStream,line) );
		REQUIRE(line.find(test_url) == 0);
		browserFileStream.close();

		REQUIRE(feed->unread_item_count() == itemCount - 1);
	}
	SECTION("OP_OPENINBROWSER") {
		itemlist.process_op(OP_OPENINBROWSER);
		std::ifstream browserFileStream (browserfile.getPath());

		REQUIRE ( std::getline (browserFileStream,line) );
		REQUIRE(line.find(test_url) == 0);
		browserFileStream.close();
	}
	SECTION("OP_OPENALLUNREADINBROWSER"){
	
		SECTION("Open all unread in browser, unread >= max-browser-tabs"){
			int maxItemsToOpen = 4;
			int openedItemsCount = 0;
			cfg.set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));

			itemlist.process_op(OP_OPENALLUNREADINBROWSER);

			std::ifstream browserFileStream (browserfile.getPath());
			openedItemsCount = 0;
			if (browserFileStream.is_open()) {
				while ( std::getline (browserFileStream,line) ) {
					INFO("Each URL should be present exactly once. Erase urls after first match to fail if an item opens twice.")
					REQUIRE(url_set.count(line) == 1);
					url_set.erase(url_set.find(line));
					openedItemsCount += 1;
				}
				browserFileStream.close();
			}
			REQUIRE(openedItemsCount == maxItemsToOpen);
		}

		SECTION("Open all unread in browser, unread < max-browser-tabs"){
			int maxItemsToOpen = 9;
			int openedItemsCount = 0;
			cfg.set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));

			itemlist.process_op(OP_OPENALLUNREADINBROWSER);

			std::ifstream browserFileStream (browserfile.getPath());
			if (browserFileStream.is_open()) {
				while ( std::getline (browserFileStream,line) ) {
					INFO("Each URL should be present exactly once. Erase urls after first match to fail if an item opens twice.")
					REQUIRE(url_set.count(line) == 1);
					url_set.erase(url_set.find(line));
					openedItemsCount += 1;
				}
				browserFileStream.close();
			}
			REQUIRE(openedItemsCount == itemCount);
		}
	}
	SECTION("OP_OPENALLUNREADINBROWSER_AND_MARK"){

		SECTION("Open all unread in browser and mark read, unread >= max-browser-tabs"){
			int maxItemsToOpen = 4;
			int openedItemsCount = 0;
			cfg.set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));

			itemlist.process_op(OP_OPENALLUNREADINBROWSER_AND_MARK);

			std::ifstream browserFileStream (browserfile.getPath());
			if (browserFileStream.is_open()) {
				while ( std::getline (browserFileStream,line) ) {
					INFO("Each URL should be present exactly once. Erase urls after first match to fail if an item opens twice.")
					REQUIRE(url_set.count(line) == 1);
					url_set.erase(url_set.find(line));
					openedItemsCount += 1;
				}
				browserFileStream.close();
			}
			REQUIRE(openedItemsCount == maxItemsToOpen);
			REQUIRE(feed->unread_item_count() == itemCount - maxItemsToOpen);
		}

		SECTION("Open all unread in browser and mark read, unread < max-browser-tabs"){
			int maxItemsToOpen = 9;
			int openedItemsCount = 0;
			cfg.set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));

			itemlist.process_op(OP_OPENALLUNREADINBROWSER_AND_MARK);

			std::ifstream browserFileStream (browserfile.getPath());
			if (browserFileStream.is_open()) {
				while ( std::getline (browserFileStream,line) ) {
					INFO("Each URL should be present exactly once. Erase urls after first match to fail if an item opens twice.")
					REQUIRE(url_set.count(line) == 1);
					url_set.erase(url_set.find(line));
					openedItemsCount += 1;
				}
				browserFileStream.close();
			}
			REQUIRE(openedItemsCount == itemCount);
			REQUIRE(feed->unread_item_count() == 0);
		}
	}
	SECTION("OP_TOGGLEITEMREAD") {
		//NOTIMPL
	}
	SECTION("OP_SHOWURLS") {
		//NOTIMPL
	}
	SECTION("OP_BOOKMARK") {
		//NOTIMPL
	}
	SECTION("OP_EDITFLAGS") {
		//NOTIMPL
	}
	SECTION("OP_SAVE") {
		//NOTIMPL
	}
	SECTION("OP_HELP") {
		//NOTIMPL
	}
	SECTION("OP_RELOAD") {
		//NOTIMPL
	}
	SECTION("OP_QUIT") {
		//NOTIMPL
	}
	SECTION("OP_HARDQUIT") {
		//NOTIMPL
	}
	SECTION("OP_NEXTUNREAD") {
		//NOTIMPL
	}
	SECTION("OP_PREVUNREAD") {
		//NOTIMPL
	}
	SECTION("OP_NEXT") {
		//NOTIMPL
	}
	SECTION("OP_PREV") {
		//NOTIMPL
	}
	SECTION("OP_RANDOMUNREAD") {
		//NOTIMPL
	}
	SECTION("OP_NEXTUNREADFEED") {
		//NOTIMPL
	}
	SECTION("OP_PREVUNREADFEED") {
		//NOTIMPL
	}
	SECTION("OP_NEXTFEED") {
		//NOTIMPL
	}
	SECTION("OP_PREVFEED") {
		//NOTIMPL
	}
	SECTION("OP_MARKFEEDREAD") {
		//NOTIMPL
	}
	SECTION("OP_TOGGLESHOWREAD") {
		//NOTIMPL
	}
	SECTION("OP_PIPE_TO") {
		//NOTIMPL
	}
	SECTION("OP_SEARCH") {
		//NOTIMPL
	}
	SECTION("OP_EDIT_URLS") {
		//NOTIMPL
	}
	SECTION("OP_SELECTFILTER") {
		//NOTIMPL
	}
	SECTION("OP_SETFILTER") {
		//NOTIMPL
	}
	SECTION("OP_CLEARFILTER") {
		//NOTIMPL
	}
	SECTION("OP_SORT") {
		//NOTIMPL
	}
	SECTION("OP_REVSORT") {
		//NOTIMPL
	}
	SECTION("OP_INT_RESIZE") {
		//NOTIMPL
	}
	SECTION("OP_INT_END_SETFILTER") {
		//NOTIMPL
	}
	SECTION("OP_INT_EDITFLAGS_END") {
		//NOTIMPL
	}
	SECTION("OP_INT_START_SEARCH") {
		//NOTIMPL
	}
	SECTION("OP_PIPE_TO") {
		//NOTIMPL
	}

}
