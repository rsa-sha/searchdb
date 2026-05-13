#include "html_parser.hpp"

#include <gumbo.h>
#include <sstream>



// -------------------- helpers --------------------

static std::string clean_text(const std::string &in) {
    std::string out;
    out.reserve(in.size());

    for (size_t i = 0; i < in.size(); i++) {
        char c = in[i];
        // basic whitespace normalization
        if (c == '\n' || c == '\t' || c == '\r') {
            out += ' ';
        } else {
            out += c;
        }
    }
    return out;
}

static void decode_entities(std::string &text) {
    // minimal version for now
    auto replace_all = [&](const std::string& from, const std::string& to) {
        size_t pos = 0;
        while ((pos = text.find(from, pos)) != std::string::npos) {
            text.replace(pos, from.length(), to);
            pos += to.length();
        }
    };

    replace_all("&amp;", "&");
    replace_all("&lt;", "<");
    replace_all("&gt;", ">");
    replace_all("&quot;", "\"");

	// numeric entities: &#NNN;
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '&' &&
            i + 2 < text.size() &&
            text[i + 1] == '#') {
            size_t semi = text.find(';', i);
            if (semi != std::string::npos) {
                std::string num =
                    text.substr(i + 2, semi - (i + 2));
                try {
                    int value = std::stoi(num);
                    if (value >= 0 && value <= 255) {
                        out += static_cast<char>(value);
                        i = semi;
                        continue;
                    }
                } catch (...) {
                    // fallthrough
                }
            }
        }
        out += text[i];
    }
    text = std::move(out);
}

// -------------------- DOM walk --------------------

static void walk(GumboNode *node, ParsedDocument &doc, bool &in_title) {
    if (!node) return;

    if (node->type == GUMBO_NODE_TEXT) {
        std::string txt = node->v.text.text;
        if (in_title) {
            doc.title += txt;
        } else {
            doc.body_text += txt;
            doc.body_text += ' ';
        }
        return;
    }

    if (node->type != GUMBO_NODE_ELEMENT) return;

    GumboTag tag = node->v.element.tag;

    // skip noisy tags
    if (tag == GUMBO_TAG_SCRIPT || tag == GUMBO_TAG_STYLE || tag == GUMBO_TAG_NOSCRIPT)
        return;

    // title handling
    bool is_title = (tag == GUMBO_TAG_TITLE);
    bool prev_title = in_title;

    if (is_title) in_title = true;

    // extract links
    if (tag == GUMBO_TAG_A) {
        GumboAttribute* href = gumbo_get_attribute(
            &node->v.element.attributes, "href"
        );

        if (href && href->value)
            doc.links.emplace_back(href->value);
    }

    // recurse children
    GumboVector* children = &node->v.element.children;
    for (size_t i = 0; i < children->length; i++)
        walk((GumboNode*)children->data[i], doc, in_title);

    if (is_title) in_title = prev_title;
}

// -------------------- public API --------------------

Result<ParsedDocument> parse_html(const std::string& html) {
    ParsedDocument doc;

    GumboOutput* output = gumbo_parse(html.c_str());
    if (!output)
        return std::unexpected(SearchError::ParseError);

    bool in_title = false;
    walk(output->root, doc, in_title);

    gumbo_destroy_output(&kGumboDefaultOptions, output);

    doc.body_text = clean_text(doc.body_text);
    decode_entities(doc.body_text);
    decode_entities(doc.title);

    return doc;
}
