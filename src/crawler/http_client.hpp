#pragma once

#include <curl/curl.h>
#include <string>
#include "../common/result.hpp"

struct HttpResponse {
	int			status_code;
	std::string body;
	std::string final_url;
	std::string content_type;
};

Result<HttpResponse> fetch(const std::string &url);
