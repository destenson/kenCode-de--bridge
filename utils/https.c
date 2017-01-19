#include <stdlib.h>
#include <string.h>

#include "https.h"

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
int utils_https_get(CURL* curl, const char* url, char** results) {
	CURLcode res;

	struct MemoryStruct chunk;

	chunk.memory = malloc(1);
	chunk.size = 0;

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
	}
	return 0;
}

/**
 * Build an escape POST line, ready to be inserted into the post transmission
 * @param curl the context for libcurl
 * @param parameters an array of structures of name/value pairs for parameters
 * @param num_parameters the numer of parameters
 * @returns a string for use by curl, which must be freed using curl_free(string)
 */
char* utils_https_build_postfield(CURL* curl, struct Parameter** parameters, int num_parameters) {
	size_t postfield_length = 0;
	for(int i = 0; i < num_parameters; i++) {
		postfield_length += strlen(parameters[i]->name);
		postfield_length += strlen(parameters[i]->value);
		postfield_length += 2; // equals and ampersands
	}

	char postfield[postfield_length];
	memset(postfield, 0, postfield_length);
	for(int i = 0; i < num_parameters; i++) {
		if (strlen(postfield) > 0)
			strcat(postfield, "&");
		strcat(postfield, parameters[i]->name);
		strcat(postfield, "=");
		strcat(postfield, parameters[i]->value);
	}
	return curl_easy_escape(curl, postfield, strlen(postfield));
}

struct HttpConnection* utils_https_new() {
	struct HttpConnection* http_connection = (struct HttpConnection*)malloc(sizeof(struct HttpConnection));
	if (http_connection == NULL)
		return NULL;
	http_connection->encoded_post_parameters = NULL;
	http_connection->headers = NULL;

	curl_global_init(CURL_GLOBAL_ALL);
	http_connection->curl = curl_easy_init();
	return http_connection;
}

void utils_https_free(struct HttpConnection* connection) {
	if (connection != NULL) {
		if (connection->curl != NULL)
			curl_easy_cleanup(connection->curl);
		if (connection->encoded_post_parameters != NULL)
			curl_free(connection->encoded_post_parameters);
		if (connection->headers != NULL)
			curl_slist_free_all(connection->headers);
		curl_global_cleanup();
	}
}

int utils_https_put(CURL* curl, const char* url, struct curl_slist* header, char* escaped_postfield, char** results) {
	CURLcode res;

	struct MemoryStruct chunk;

	chunk.memory = malloc(1);
	chunk.size = 0;

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, escaped_postfield);
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
	}
	return 0;
}
