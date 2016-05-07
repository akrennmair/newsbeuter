#ifndef LISTFORMATTER__H
#define LISTFORMATTER__H

#include <climits>
#include <vector>
#include <string>
#include <utility>
#include <regexmanager.h>

namespace newsbeuter {


class listformatter {

		typedef std::pair<std::string, unsigned int> line_id_pair;

	public:
		listformatter();
		~listformatter();
		void add_line(const std::string& text, unsigned int id = UINT_MAX);
		void add_lines(const std::vector<std::string>& lines);
		void set_line(const unsigned int itempos, const std::string& text,
		    unsigned int id = UINT_MAX);
		inline void clear() { lines.clear(); }
		std::string format_list(regexmanager * r = NULL,
		    const std::string& location = "",
			const unsigned int& width = 0);
		inline unsigned int get_lines_count() {
			return lines.size();
		}
	private:
		std::vector<std::string> wrap_line(std::string line, unsigned int width);
		std::vector<line_id_pair> lines;
		std::string format_cache;
};

}

#endif
