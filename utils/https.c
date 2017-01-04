#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct MemoryStruct {
	char* memory;
	size_t size;
};

static size_t write_memory_callback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		/* out of memory! */
	    return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

/***
 * Send an HTTPS GET and receive the results
 * @param url the complete url (i.e. 'https://myserver.org/market/getmarket?pair=BTCUSD&nonce=....')
 * @param results where to store the results. NOTE: memory will be allocated
 * @returns 0 on success, otherwise a negative number that denotes the error
 */
int utils_https_get(const char* url, char** results) {
	CURL* curl;
	CURLcode res;

	struct MemoryStruct chunk;

	chunk.memory = malloc(1);
	chunk.size = 0;

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			return -1;
		} else {
			*results = chunk.memory;
		}
		curl_easy_cleanup(curl);
		curl_global_cleanup();
	}
	return 0;
}
