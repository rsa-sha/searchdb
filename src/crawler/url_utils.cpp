#include "url_utils.hpp"

#include <algorithm>
#include <sstream>
#include <vector>

struct ParsedUrl {
	std::string scheme;
	std::string authority;
	std::string host;
	std::string port;
	std::string path;
	std::string query;
	std::string fragment;
};

static bool is_http_scheme(const std::string &scheme) {
	return scheme == "http" || scheme == "https";
}

static std::string to_lower(std::string s) {
    std::transform(
        s.begin(),
        s.end(),
        s.begin(),
        [](unsigned char c) {
            return std::tolower(c);
        }
    );

    return s;
}

static bool is_unreserved(char c) {
	return std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '.' || c == '~' || c == '_';
}

static int hex_to_int(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    c = std::tolower(c);

    if (c >= 'a' && c <= 'f')
        return 10 + (c - 'a');
    return -1;
}

static std::string percent_decode_unreserved(const std::string &input) {
    std::string out;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '%' && i + 2 < input.size()) {
            int ha = hex_to_int(input[i + 1]);
            int hb = hex_to_int(input[i + 2]);
            if (ha != -1 && hb != -1) {
				// hex =  ha*16 + hb
                char decoded = static_cast<char>((ha<< 4) | hb);
                if (is_unreserved(decoded)) {
                    out += decoded;
                    i += 2;
					continue;
                }
            }
        }
        out += input[i];
    }
    return out;
}

static std::string build_url(const ParsedUrl &url) {
    std::string out;

    out += url.scheme;
    out += "://";
    out += url.host;
    if (!url.port.empty()) {
        out += ":";
        out += url.port;
    }
    out += url.path;
    if (!url.query.empty()) {
        out += "?";
        out += url.query;
    }
    return out;
}

static std::string remove_dot_segments(const std::string& path) {
    std::vector<std::string> stack;

    std::stringstream ss(path);
    std::string segment;

    while (std::getline(ss, segment, '/')) {
        if (segment.empty() || segment == ".")
            continue;
        if (segment == "..") {
            if (!stack.empty())
                stack.pop_back();
        } else {
            stack.push_back(segment);
        }
    }
    std::string out = "/";
    for (size_t i = 0; i < stack.size(); ++i) {
        out += stack[i];
        if (i + 1 < stack.size())
            out += "/";
    }
    return out;
}

static std::optional<ParsedUrl> url_component_extraction (const std::string &url) {
	// For now we are only supporting http, https
	ParsedUrl components;
	auto scheme_end = url.find("://");

	if (scheme_end == std::string::npos)
		return std::nullopt;

	components.scheme = to_lower(url.substr(0, scheme_end));

	if (!is_http_scheme(components.scheme))
		return std::nullopt;

	size_t authority_begin = scheme_end + 3; // '://'

	// stop before occurence of rel_path, query or fragment
	size_t authority_end = url.find_first_of("/?#", authority_begin);

	if (authority_end == std::string::npos)
		authority_end = url.size();

	std::string authority = url.substr(authority_begin, authority_end - authority_begin);

	if (authority.empty())
		return std::nullopt;

	auto colon = authority.find(":");

	if (colon != std::string::npos) {
		components.host = to_lower(authority.substr(0, colon));
		components.port = authority.substr(colon+1);
	} else {
		components.host = to_lower(authority);
	}
	components.authority = authority;


	size_t pos = authority_end;

	// path
	if (pos < url.size() && url[pos] == '/') {
		size_t path_end = url.find_first_of("?#", pos);
		if (path_end == std::string::npos)
			path_end = url.size();
	
		components.path = url.substr(pos, path_end - pos);
		pos = path_end;
	}
	if (components.path.empty())
		components.path = "/";

	// query
	if (pos <url.size() && url[pos] == '?') {
		size_t query_end = url.find("#", pos);
		if (query_end == std::string::npos)
			query_end = url.size();
		components.query = url.substr(pos + 1, query_end - pos - 1);
		pos = query_end;
	}

	// fragment
	if (pos < url.size() && url[pos] == '#')
		components.fragment = url.substr(pos+1);

	return components;
}


std::string normalize_url(const std::string &url) {
	auto parsed_url = url_component_extraction(url);
	if (!parsed_url)
		return "";
	// removing default ports
	if ((parsed_url->scheme == "http" && parsed_url->port == "80") || 
		(parsed_url->scheme == "https" && parsed_url->port == "443"))
		parsed_url->port.clear();

	// remove fragments
	parsed_url->fragment.clear();

	// decoding unreserved characters [RFC3986]
	parsed_url->path = percent_decode_unreserved(parsed_url->path);

	// remove trailing slash
	if (parsed_url->path.size() > 1 && parsed_url->path.back() == '/')
		parsed_url->path.pop_back();

	return build_url(*parsed_url);
}

std::string extract_domain(const std::string &url) {
	auto parsed_url = url_component_extraction(url);

	if (!parsed_url)
		return "";
	return parsed_url->host;
}

std::string resolve_url(const std::string &base, const std::string &relative) {
	if (relative.starts_with("http://") || relative.starts_with("https://"))
		return normalize_url(relative);

	auto base_url = url_component_extraction(base);
	if (!base_url)
		return "";

	// protocol-relative
	if (relative.starts_with("//"))
		return normalize_url(base_url->scheme+":"+relative);

	ParsedUrl out = *base_url;

    // root-relative
    if (!relative.empty() && relative[0] == '/') {
        out.path = remove_dot_segments(relative);
        out.query.clear();
        out.fragment.clear();
        return normalize_url(build_url(out));
    }

    // relative path
    auto last_slash =
        out.path.find_last_of('/');

    std::string base_dir;

    if (last_slash != std::string::npos) {
        base_dir =
            out.path.substr(0, last_slash + 1);
    } else {
        base_dir = "/";
    }

    out.path =
        remove_dot_segments(
            base_dir + relative
        );

    out.query.clear();
    out.fragment.clear();

    return normalize_url(build_url(out));
}
