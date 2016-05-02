#include <listformatter.h>
#include <utils.h>
#include <stflpp.h>
#include <assert.h>
#include <limits.h>

namespace newsbeuter {

listformatter::listformatter() : refresh_cache(true) { }

listformatter::~listformatter() { }

void listformatter::add_line(const std::string& text, unsigned int id, unsigned int width) {
	set_line(UINT_MAX, text, id, width);
	LOG(LOG_DEBUG, "listformatter::add_line: `%s'", text.c_str());
	refresh_cache = true;
}

void listformatter::set_line(const unsigned int itempos,
    const std::string& text, unsigned int id, unsigned int width)
{
	std::vector<line_id_pair> formatted_text;

	if (width > 0 && text.length() > 0) {
		std::wstring mytext = utils::clean_nonprintable_characters(utils::str2wstr(text));

		while (mytext.length() > 0) {
			size_t size = mytext.length();
			size_t w = utils::wcswidth_stfl(mytext, size);
			if (w > width) {
				while (size && (w = utils::wcswidth_stfl(mytext, size)) > width) {
					size--;
				}
			}
			formatted_text.push_back(
					line_id_pair(utils::wstr2str(mytext.substr(0, size)), id));
			mytext.erase(0, size);
		}
	} else {
		formatted_text.push_back(
				line_id_pair(
					utils::wstr2str(
						utils::clean_nonprintable_characters(
							utils::str2wstr(text))),
					id));
	}

	if (itempos == UINT_MAX) {
		lines.insert(
				lines.cend(),
				formatted_text.cbegin(),
				formatted_text.cend());
	} else {
		lines[itempos] = formatted_text[0];
	}
}

void listformatter::add_lines(const std::vector<std::string>& thelines, unsigned int width) {
	for (auto line : thelines) {
		add_line(utils::replace_all(line, "\t", "        "), UINT_MAX, width);
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
	refresh_cache = false;
	return format_cache;
}

std::vector<std::string> listformatter::wrap_line(std::string line, unsigned int width) {
	std::vector<std::string> lines;
	std::string::size_type pos;
	LOG(LOG_DEBUG, "listformatter::wrap_line: `%s'", line.c_str());

	while (line.length() > width) {
		unsigned int i = width;
		while (i > 0 && line[i] != ' ' && line[i] != '<')
			--i;
		if (line[i] == '<') {
			i = width;
			while ((i < line.length()) && (line[i] != '>'))
				++i;
			++i;
		}
		if (0 == i) {
			i = width;
		}
		std::string subline = line.substr(0, i);
		line.erase(0, i);
		line.erase(0, line.find_first_not_of(" "));
		subline.erase(0, subline.find_first_not_of(" "));
		if(subline.length() > 0) {
			LOG(LOG_DEBUG, "listformatter::wrap_line: --> `%s'", subline.c_str());
			lines.push_back(subline);
		}
	}

	if ((line.length() > 0) || (lines.size() == 0)) {
			LOG(LOG_DEBUG, "listformatter::wrap_line: --> `%s'", line.c_str());
			lines.push_back(line);
	}
	return lines;
}

}
