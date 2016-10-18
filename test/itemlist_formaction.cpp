#include "catch.hpp"

#include "cache.h"
#include "itemlist_formaction.h"
#include "keymap.h"
#include "stfl/itemlist.h"

#include <unistd.h>

using namespace newsbeuter;

TEST_CASE("itemlist_formaction::process_op()") {

	controller c;
	newsbeuter::view v(&c);
	std::string test_browser_file = "/tmp/test_browser.txt";
	std::string test_url = "http://test_url";
	std::string cache_path = "test-cache.db";
	std::string line;
	int itemCount = 6;
	int openedItemsCount = 0;
	int lineCount = 0;
	int maxItems = 4;
	int openedItemsTarget = 0;

	configcontainer * cfg = new configcontainer();
	cfg->set_configvalue("browser", "echo %u >> " + test_browser_file);
	cfg->set_configvalue("max-browser-tabs", std::to_string(maxItems));

	cache * rsscache = new cache(cache_path, cfg);

	std::shared_ptr<rss_feed> feed(new rss_feed(rsscache));

	for (int i = 0; i < itemCount; i++)
	{
		std::shared_ptr<rss_item> item(new rss_item(rsscache));

		item->set_link(test_url + std::to_string(i));
		item->set_unread(true);

		feed->add_item(item);
	}

	v.set_config_container(cfg);
	c.set_view(&v);

	std::shared_ptr<itemlist_formaction> itemlist(new itemlist_formaction(&v, itemlist_str));
	itemlist->set_feed(feed);

	SECTION("Itemlist Formaction"){
	
		SECTION("Open all unread in browser, unread > max-browser-tabs"){
			openedItemsTarget = maxItems;
			itemlist->process_op(OP_OPENALLUNREADINBROWSER);

			std::ifstream myfile (test_browser_file);
			openedItemsCount = 0;
  			if (myfile.is_open())
  			{
    				for (int i = 0; i < maxItems; i ++)
    				{
					if ( std::getline (myfile,line) )
					{
						REQUIRE(line == test_url + std::to_string(i));
						openedItemsCount += 1;
					}
    				}
    				myfile.close();
			}
			REQUIRE(openedItemsCount == openedItemsTarget);
			std::remove(test_browser_file.c_str());
		}

		SECTION("Open all unread in browser, unread < max-browser-tabs"){
			maxItems = 9;
			openedItemsCount = 0;
			openedItemsTarget = itemCount;
			cfg->set_configvalue("max-browser-tabs", std::to_string(maxItems));
			itemlist->process_op(OP_OPENALLUNREADINBROWSER);

			std::ifstream myfile (test_browser_file);
  			if (myfile.is_open())
  			{
    				for (int i = 0; i < itemCount; i ++)
    				{
					if ( std::getline (myfile,line) )
					{
						REQUIRE(line == test_url + std::to_string(i));
						openedItemsCount += 1;
					}
    				}
    				myfile.close();
			}
			REQUIRE(openedItemsCount == openedItemsTarget);
			std::remove(test_browser_file.c_str());
		}

		SECTION("Open all unread in browser and mark read, unread > max-browser-tabs"){
			maxItems = 4;
			openedItemsCount = 0;
			openedItemsTarget = maxItems;
			cfg->set_configvalue("max-browser-tabs", std::to_string(maxItems));
			itemlist->process_op(OP_OPENALLUNREADINBROWSER_AND_MARK);

			std::ifstream myfile (test_browser_file);
  			if (myfile.is_open())
  			{
    				for (int i = 0; i < itemCount; i ++)
    				{
					if ( std::getline (myfile,line) )
					{
						REQUIRE(line == test_url + std::to_string(i));
						openedItemsCount += 1;
					}
    				}
    				myfile.close();
			}
			REQUIRE(openedItemsCount == openedItemsTarget);
			REQUIRE(feed->unread_item_count() == itemCount - maxItems);
			std::remove(test_browser_file.c_str());
		}

		SECTION("Open all unread in browser and mark read, unread < max-browser-tabs"){
			openedItemsCount = 0;
			openedItemsTarget = feed->unread_item_count();
			maxItems = 9;
			cfg->set_configvalue("max-browser-tabs", std::to_string(maxItems));
			itemlist->process_op(OP_OPENALLUNREADINBROWSER_AND_MARK);

			std::ifstream myfile (test_browser_file);
  			if (myfile.is_open())
  			{
    				for (int i = 0; i < itemCount; i ++)
    				{
					if ( std::getline (myfile,line) )
					{
						REQUIRE(line == test_url + std::to_string(i));
						openedItemsCount += 1;
					}
    				}
    				myfile.close();
			}
			REQUIRE(openedItemsCount == openedItemsTarget);
			REQUIRE(feed->unread_item_count() == 0);
			std::remove(test_browser_file.c_str());
		}
	}

	delete cfg;
	delete rsscache;


	::unlink("test-cache.db");
}
