#ifndef XMLPULLPARSER_H_
#define XMLPULLPARSER_H_

#include <string>
#include <utility>
#include <vector>
#include <list>

namespace noos
{

typedef std::pair<std::string,std::string> xmlattribute;

enum event { START_DOCUMENT, END_DOCUMENT, START_TAG, END_TAG, TEXT };

struct xmlnode {
	std::vector<xmlattribute> attributes;
	std::string text;
	event ev;
	void add_attribute(std::string s);
	event determine_tag_type();
};

class xmlpullparser
{
public:
	
	xmlpullparser();
	virtual ~xmlpullparser();
	void setInput(std::istream& is);
	int getAttributeCount() const;
	std::string getAttributeName(unsigned int index) const;
	std::string getAttributeValue(unsigned int index) const;
	std::string getAttributeValue(const std::string& name) const;
	event getEventType() const;
	std::string getText() const;
	bool isWhitespace() const;
	event next();
	
private:
	std::istream * inputstream;
	std::list<xmlnode> nodequeue;

	void parse_next();
	
	int skip_whitespace();
	std::string read_tag();
	std::string decode_attribute(const std::string& s);
	std::string decode_entities(const std::string& s);
	std::string decode_entity(std::string s);
	void remove_trailing_whitespace(std::string& s);
	
};

}

#endif /*XMLPULLPARSER_H_*/
