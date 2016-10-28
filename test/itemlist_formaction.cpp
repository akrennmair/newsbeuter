#include "catch.hpp"

#include <cache.h>
#include <itemlist_formaction.h>
#include <keymap.h>
#include <stfl/itemlist.h>
#include "test-helpers.h"

#include <unistd.h>

using namespace newsbeuter;

TEST_CASE("itemlist_formaction::process_op()") {
	controller c;
	newsbeuter::view v(&c);
	TestHelpers::TempFile browserfile;

	std::string test_url = "http://test_url";
	std::string line;
	int itemCount = 6;

	std::shared_ptr<configcontainer> cfg ( new configcontainer());
	cfg->set_configvalue("browser", "echo %u >> " + browserfile.getPath());

	std::unique_ptr<cache> rsscache(new cache(":memory:", cfg.get()));

	std::shared_ptr<rss_feed> feed(new rss_feed(rsscache.get()));

	for (int i = 0; i < itemCount; i++) {
		std::shared_ptr<rss_item> item(new rss_item(rsscache.get()));

		item->set_link(test_url + std::to_string(i));
		item->set_unread(true);

		feed->add_item(item);
	}

	v.set_config_container(cfg.get());
	c.set_view(&v);

	std::unique_ptr<itemlist_formaction> itemlist(new itemlist_formaction(&v, itemlist_str));
	itemlist->set_feed(feed);

	SECTION("Itemlist Formaction"){
	
		SECTION("Open all unread in browser, unread >= max-browser-tabs"){
			int maxItemsToOpen = 4;
			int openedItemsCount = 0;
			cfg->set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));
			
			itemlist->process_op(OP_OPENALLUNREADINBROWSER);

			std::ifstream browserFileStream (browserfile.getPath());
			openedItemsCount = 0;
			if (browserFileStream.is_open()) {
				for (int i = 0; i < maxItemsToOpen; i ++) {
					if ( std::getline (browserFileStream,line) ) {
						REQUIRE(line == test_url + std::to_string(i));
						openedItemsCount += 1;
					}
				}
				browserFileStream.close();
			}
			REQUIRE(openedItemsCount == maxItemsToOpen);
		}

		SECTION("Open all unread in browser, unread < max-browser-tabs"){
			int maxItemsToOpen = 9;
			int openedItemsCount = 0;
			cfg->set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));

			itemlist->process_op(OP_OPENALLUNREADINBROWSER);

			std::ifstream browserFileStream (browserfile.getPath());
			if (browserFileStream.is_open()) {
				for (int i = 0; i < itemCount; i ++) {
					if ( std::getline (browserFileStream,line) ) {
						REQUIRE(line == test_url + std::to_string(i));
						openedItemsCount += 1;
					}
				}
				browserFileStream.close();
			}
			REQUIRE(openedItemsCount == itemCount);
		}

		SECTION("Open all unread in browser and mark read, unread >= max-browser-tabs"){
			int maxItemsToOpen = 4;
			int openedItemsCount = 0;
			cfg->set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));

			itemlist->process_op(OP_OPENALLUNREADINBROWSER_AND_MARK);

			std::ifstream browserFileStream (browserfile.getPath());
			if (browserFileStream.is_open()) {
				for (int i = 0; i < itemCount; i ++) {
					if ( std::getline (browserFileStream,line) ) {
						REQUIRE(line == test_url + std::to_string(i));
						openedItemsCount += 1;
					}
				}
				browserFileStream.close();
			}
			REQUIRE(openedItemsCount == maxItemsToOpen);
			REQUIRE(feed->unread_item_count() == itemCount - maxItemsToOpen);
		}

		SECTION("Open all unread in browser and mark read, unread < max-browser-tabs"){
			int maxItemsToOpen = 9;
			int openedItemsCount = 0;
			cfg->set_configvalue("max-browser-tabs", std::to_string(maxItemsToOpen));

			itemlist->process_op(OP_OPENALLUNREADINBROWSER_AND_MARK);

			std::ifstream browserFileStream (browserfile.getPath());
			if (browserFileStream.is_open()) {
				for (int i = 0; i < itemCount; i ++) {
					if ( std::getline (browserFileStream,line) ) {
						REQUIRE(line == test_url + std::to_string(i));
						openedItemsCount += 1;
					}
				}
				browserFileStream.close();
			}
			REQUIRE(openedItemsCount == itemCount);
			REQUIRE(feed->unread_item_count() == 0);
		}
	}

}
