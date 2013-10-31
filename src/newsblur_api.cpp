#include <vector>
#include <iostream>
#include <wordexp.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include <newsblur_api.h>
#include <utils.h>
#include <logger.h>
#include <curl/curl.h>
#include <json.h>

#define NEWSBLUR_LOGIN "http://www.newsblur.com/api/login"
#define NEWSBLUR_READER_FEEDS "http://www.newsblur.com/reader/feeds"

namespace newsbeuter {

newsblur_api::newsblur_api(configcontainer * c) : remote_api(c) {
	char tmpl[256] = "/tmp/newsblur-cookiejar.XXXXXX";
	mkstemp(tmpl);
	cookiejar = tmpl;
	LOG(LOG_DEBUG, "newsblur_api: cookie jar = %s", cookiejar.c_str());
}

newsblur_api::~newsblur_api() {
	unlink(cookiejar.c_str());
}

bool newsblur_api::authenticate() {
	auth = retrieve_auth();
	LOG(LOG_DEBUG, "newsblur_api::authenticate: Auth = %s", auth.c_str());
	return auth != "";
}

static size_t my_write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
	std::string * pbuf = static_cast<std::string *>(userp);
	pbuf->append(static_cast<const char *>(buffer), size * nmemb);
	return size * nmemb;
}

std::string newsblur_api::retrieve_auth() {
	std::string user = cfg->get_configvalue("newsblur-login");
	bool flushed = false;

	if (user == "") {
		std::cout << std::endl;
		std::cout.flush();
		flushed = true;
		std::cout << "Username for Newsblur: ";
		std::cin >> user;
		if (user == "") {
			return "";
		}
	}

	std::string pass = cfg->get_configvalue("newsblur-password");
	if (pass == "") {
		wordexp_t exp;
		std::ifstream ifs;
		wordexp(cfg->get_configvalue("oldreader-passwordfile").c_str(),&exp,0);
		ifs.open(exp.we_wordv[0]); 
		wordfree(&exp);
		if (!ifs) {
			if(!flushed) {
				std::cout << std::endl;
				std::cout.flush();
			}
			// Find a way to do this in C++ by removing cin echoing.
			pass = std::string( getpass("Password for The Old Reader: ") );
		} else {
			ifs >> pass;
			if (pass == "") {
				return "";
			}
		}
	}

	CURL * handle = curl_easy_init();
	char * username = curl_easy_escape(handle, user.c_str(), 0);
	char * password = curl_easy_escape(handle, pass.c_str(), 0);

	std::string postcontent = utils::strprintf("username=%s&password=%s", username, password);

	curl_free(username);
	curl_free(password);

	std::string result;

	utils::set_common_curl_options(handle, cfg);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, my_write_data);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &result);
	curl_easy_setopt(handle, CURLOPT_POSTFIELDS, postcontent.c_str());
	curl_easy_setopt(handle, CURLOPT_URL, NEWSBLUR_LOGIN);
	curl_easy_setopt(handle, CURLOPT_COOKIEFILE, cookiejar.c_str());
	curl_easy_setopt(handle, CURLOPT_COOKIEJAR, cookiejar.c_str());
	CURLcode code = curl_easy_perform(handle);
	curl_easy_cleanup(handle);

	LOG(LOG_DEBUG, "newsblur_api::retrieve_auth: result = %s (%s)", result.c_str(), curl_easy_strerror(code));

	struct json_object * reply = json_tokener_parse(result.c_str());
	if (is_error(reply)) {
		LOG(LOG_DEBUG, "newsblur_api::retrieve_auth: couldn't parse server response.");
		return "";
	}

	json_bool authenticated = json_object_get_boolean(json_object_object_get(reply, "authenticated"));

	json_object_put(reply);

	return authenticated ? "newsblur-authenticated" : "";
}

std::vector<tagged_feedurl> newsblur_api::get_subscribed_urls() {
	std::vector<tagged_feedurl> urls;

	std::string result;

	CURL * handle = curl_easy_init();
	utils::set_common_curl_options(handle, cfg);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, my_write_data);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &result);
	curl_easy_setopt(handle, CURLOPT_URL, NEWSBLUR_READER_FEEDS);
	curl_easy_setopt(handle, CURLOPT_COOKIEFILE, cookiejar.c_str());
	curl_easy_setopt(handle, CURLOPT_COOKIEJAR, cookiejar.c_str());
	CURLcode code = curl_easy_perform(handle);
	curl_easy_cleanup(handle);

	LOG(LOG_DEBUG, "newsblur_api::get_subscribed_urls: result = %s (%s)", result.c_str(), curl_easy_strerror(code));

	return urls;
}

bool newsblur_api::mark_all_read(const std::string& feedurl) {
	// TODO: implement
	return false;
}

bool newsblur_api::mark_article_read(const std::string& guid, bool read) {
	// TODO: implement
	return false;
}

bool newsblur_api::update_article_flags(const std::string& oldflags, const std::string& newflags, const std::string& guid) {
	// TODO: implement
	return false;
}

void newsblur_api::configure_handle(CURL *) {
	// nothing.
}

}
