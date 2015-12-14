#include <newsblur_api.h>
#include <logger.h>

namespace newsbeuter {

newsblur_urlreader::newsblur_urlreader(const std::string& url_file, remote_api * a) : file(url_file), api(a) { }

newsblur_urlreader::~newsblur_urlreader() { }

void newsblur_urlreader::write_config() {
	// NOTHING
}

void newsblur_urlreader::dump_urls_to(const std::string& filepath) {
	LOG(LOG_ERROR, "newsblur_urlreader::dump_urls_to: not implemented");
}

void newsblur_urlreader::reload() {
	urls.clear();
	tags.clear();
	alltags.clear();

	file_urlreader ur(file);
	ur.reload();

	std::vector<std::string>& file_urls(ur.get_urls());
	for (auto url : file_urls) {
		if (url.substr(0,6) == "query:") {
			urls.push_back(url);
		}
	}

	std::vector<tagged_feedurl> feedurls = api->get_subscribed_urls();

	for (auto url : feedurls) {
		LOG(LOG_INFO, "added %s to URL list", url.first.c_str());
		urls.push_back(url.first);
		tags[url.first] = url.second;
		for (auto tag : url.second) {
			LOG(LOG_DEBUG, "%s: added tag %s", url.first.c_str(), tag.c_str());
			alltags.insert(tag);
		}
	}
}

std::string newsblur_urlreader::get_source() {
	return "NewsBlur";
}
// TODO

}
