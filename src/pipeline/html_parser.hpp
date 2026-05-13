#pragma once


#include "../common/result.hpp"

#include <gumbo.h>
#include <string>
#include <vector>


struct ParsedDocument {
    std::string title;       // from <title> tag
    std::string body_text;   // all text from <body>, space-separated
    std::vector<std::string> links;  // href values from <a> tags
};

Result<ParsedDocument> parse_html(const std::string &html);
