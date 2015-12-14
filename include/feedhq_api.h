#ifndef NEWSBEUTER_FEEDHQ_API__H
#define NEWSBEUTER_FEEDHQ_API__H

#include <remote_api.h>
#include <urlreader.h>
#include <cache.h>

namespace newsbeuter {

class feedhq_api : public remote_api {
	public:
		feedhq_api(configcontainer * c);
		virtual ~feedhq_api();
		virtual bool authenticate();
		virtual std::vector<tagged_feedurl> get_subscribed_urls();
		virtual void configure_handle(CURL * handle);
		virtual bool mark_all_read(const std::string& feedurl);
		virtual bool mark_article_read(const std::string& guid, bool read);
		virtual bool update_article_flags(const std::string& oldflags, const std::string& newflags, const std::string& guid);
		virtual bool subscribe_to_feed(const std::string& feedurl);
		virtual bool unsubscribe_from_feed(const std::string& feedurl);
		std::vector<std::string> bulk_mark_articles_read(const std::vector<google_replay_pair>& actions);
	private:
		std::vector<std::string> get_tags(xmlNode * node);
		std::string get_new_token();
		std::string retrieve_auth();
		std::string post_content(const std::string& url, const std::string& postdata);
		bool star_article(const std::string& guid, bool star);
		bool share_article(const std::string& guid, bool share);
		bool mark_article_read_with_token(const std::string& guid, bool read, const std::string& token);
		std::string auth;
};

class feedhq_urlreader : public urlreader {
	public:
		feedhq_urlreader(configcontainer * c, const std::string& url_file, remote_api * a);
		virtual ~feedhq_urlreader();
		virtual void write_config();
		virtual void dump_urls_to(const std::string& filepath);
		virtual void reload();
		virtual std::string get_source();
	private:
		configcontainer * cfg;
		std::string file;
		remote_api * api;

};

}

#endif
