#include "catch.hpp"

#include <cache.h>
#include <feedlist_formaction.h>
#include <keymap.h>
#include <stfl/feedlist.h>
#include "test-helpers.h"

#include <unistd.h>

using namespace newsbeuter;

TEST_CASE("feedlist_formaction::process_op()") {
	controller c;
	newsbeuter::view v(&c);
	TestHelpers::TempFile browserfile;
	std::string test_url = "http://test_url";



	std::shared_ptr<configcontainer> cfg(new configcontainer());
	cfg->set_configvalue("browser", "echo %u >> " + browserfile.getPath());

	std::vector<std::shared_ptr<rss_feed>> feeds;

	std::unique_ptr<cache> rsscache (new cache(":memory:", cfg.get()));

	std::shared_ptr<rss_feed> feed(new rss_feed(rsscache.get()));

	std::shared_ptr<rss_item> item(new rss_item(rsscache.get()));

	item->set_link(test_url);
	item->set_unread(true);

	feed->add_item(item);

	feeds.push_back(feed);

	v.set_config_container(cfg.get());
	c.set_view(&v);

	std::unique_ptr<feedlist_formaction> feedlist(new feedlist_formaction(&v, feedlist_str));
	feedlist->set_feedlist(feeds);
	feedlist->get_form()->set("feedposname", "0");
	
	feedlist->process_op(OP_OPENALLUNREADINBROWSER);

}
