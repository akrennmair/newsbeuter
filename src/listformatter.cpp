#include <listformatter.h>
#include <utils.h>
#include <stflpp.h>
#include <assert.h>
#include <limits.h>

namespace newsbeuter {

listformatter::listformatter() { }

listformatter::~listformatter() { }

void listformatter::add_line(const std::string& text, unsigned int id) {
	set_line(UINT_MAX, text, id);
	LOG(LOG_DEBUG, "listformatter::add_line: `%s'", text.c_str());
}

void listformatter::set_line(const unsigned int itempos,
    const std::string& text, unsigned int id)
{
	line_id_pair line = line_id_pair(
						    utils::wstr2str(
							    utils::clean_nonprintable_characters(
								    utils::str2wstr(text))),
							id);

	if (itempos == UINT_MAX) {
		lines.push_back(line);
	} else {
		lines[itempos] = line;
	}
}

void listformatter::add_lines(const std::vector<std::string>& thelines) {
	for (auto line : thelines) {
		add_line(utils::replace_all(line, "\t", "        "), UINT_MAX);
	}
}

std::string listformatter::format_list(regexmanager * rxman, const std::string& location, const unsigned int &width) {
	format_cache = "{list";
	for (auto line : lines) {
		std::string str = line.first;

		if (rxman)
			rxman->quote_and_highlight(str, location);

		std::vector<std::string> wrapped_lines;
		if (width > 0)
			wrapped_lines = wrap_line(str, width);
		else
			wrapped_lines.push_back(str);

		for (auto substr : wrapped_lines) {
			if (line.second == UINT_MAX) {
				format_cache.append(utils::strprintf("{listitem text:%s}", stfl::quote(substr).c_str()));
			} else {
				format_cache.append(utils::strprintf("{listitem[%u] text:%s}", line.second, stfl::quote(substr).c_str()));
			}
		}
	}
	format_cache.append(1, '}');
	return format_cache;
}

std::vector<std::string> listformatter::wrap_line(std::string line, unsigned int width) {

	std::vector<std::string> lines;
	std::vector<std::string> words = utils::tokenize_spaced(line);
	std::string subline = "";

	for( auto word : words ){
		unsigned int wlength = utils::strwidth_stfl(word);
		unsigned int llength = utils::strwidth_stfl(subline);

		while (wlength >= width) {
			unsigned int avail_length = width - llength;
			if (avail_length == 0) {
				lines.push_back(subline);
				subline = "";
				avail_length = width;
			}
			subline.append(word.substr(0, avail_length));
			word.erase(0, avail_length);
			lines.push_back(subline);
			subline = "";

			wlength = utils::strwidth_stfl(word);
			llength = utils::strwidth_stfl(subline);
		}
		if ((llength + wlength) > width) {
			lines.push_back(subline);
			if (word == " ")
				word = "";
			subline = word;
		} else {
			subline.append(word);
		}
	}

	if (subline.length() > 0)
		lines.push_back(subline);

	return lines;
}

}
