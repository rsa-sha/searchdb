#include "http_client.hpp"

static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	size_t total = size * nmemb;

	auto* body = static_cast<std::string*>(userdata);
	body->append(ptr, total);

	return total;
}


Result<HttpResponse> fetch(const std::string &url) {
	CURL* curl_obj = curl_easy_init();
	if (!curl_obj)
		return std::unexpected(SearchError::MemError);

	HttpResponse resp;

	curl_easy_setopt(curl_obj, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_obj, CURLOPT_FOLLOWLOCATION, 1l);			// Redir to new site on 30X
	curl_easy_setopt(curl_obj, CURLOPT_MAXREDIRS, 5l);				// Maximum 5 redirections
	curl_easy_setopt(curl_obj, CURLOPT_TIMEOUT, 10L);				// 10 sec timeout limit
	curl_easy_setopt(curl_obj, CURLOPT_USERAGENT, "searchdb/v0.1");
	curl_easy_setopt(curl_obj, CURLOPT_MAXFILESIZE, 5L*1024*1024);	// Max file size set to 5 MiB

	// write callback
	curl_easy_setopt(curl_obj, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl_obj, CURLOPT_WRITEDATA, &resp.body);

	CURLcode ret_code = curl_easy_perform(curl_obj);

	if (ret_code != CURLE_OK) {
		curl_easy_cleanup(curl_obj);
		return std::unexpected(SearchError::ParseError);
	}

	curl_easy_getinfo(curl_obj, CURLINFO_RESPONSE_CODE, &resp.status_code);

	char* final_url = nullptr;
	curl_easy_getinfo(curl_obj, CURLINFO_EFFECTIVE_URL, &final_url);
	if (final_url)
		resp.final_url = final_url;

	char* content_type = nullptr;
	curl_easy_getinfo(curl_obj, CURLINFO_CONTENT_TYPE, &content_type);
	if (content_type)
		resp.content_type = content_type;

	curl_easy_cleanup(curl_obj);
	return resp;
}
