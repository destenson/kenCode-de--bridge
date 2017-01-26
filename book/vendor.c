#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "bridge/book.h"
#include "utils/khash.h"
#include "utils/logging.h"

/**
 * Functions to handle Vendor and VendorList manipulation
 */

/**
 * Given the list of vendors, get all markets available. NOTE: Many areas of the struct are not set. Only the two currencies and the fee
 * @param vendor_head a linked list of vendors
 * @returns a Market linked list of all pairs available from the list of vendors, with duplicates removed
 */
const int khash_market = 33;
KHASH_MAP_INIT_STR(khash_market, struct Market*);
struct Market* vendor_get_all_trading_pairs(struct VendorList* vendor_head) {
	struct KeyDelete {
		char* key;
		struct KeyDelete* next;
	};
	struct KeyDelete* first_key = NULL;
	struct KeyDelete* current_key = NULL;
	// build a hashmap of valid markets
	khash_t(khash_market) *hashtable = kh_init(khash_market);
	logit(LOGLEVEL_DEBUG, "Hashtable built");
	struct VendorList* current = vendor_head;
	while (current != NULL) {
		while(!current->vendor->IsInitialized) {
			logit_string(LOGLEVEL_DEBUG, "Waiting for initialization of vendor %s", current->vendor->Name);
			sleep(1);
		}
		struct Market* market_head = current->vendor->current_market;
		logit(LOGLEVEL_DEBUG, "Have next vendor");
		struct Market* current_market = market_head;
		while (current_market != NULL) {
			logit(LOGLEVEL_DEBUG, "Have next market");
			int ret;
			size_t str_len = strlen(current_market->base_currency) + strlen(current_market->market_currency) + 2;
			char* key = (char*)malloc(str_len);
			memset(key, 0, str_len);
			sprintf(key, "%s~%s", current_market->base_currency, current_market->market_currency);
			logit_string(LOGLEVEL_DEBUG, "hashtable key: %s", key);
			khiter_t i = kh_put(khash_market, hashtable, key, &ret);
			logit_int(LOGLEVEL_DEBUG, "kh_put returned: %d", ret);
			if (ret == 1) {
				// not a duplicate key
				struct Market* market = market_new();
				logit_int(LOGLEVEL_DEBUG, "New market successful (1=yes)? %d", market != NULL);
				market->base_currency = strdup(current_market->base_currency);
				market->market_currency = strdup(current_market->market_currency);
				market->fee = 0.01;
				logit(LOGLEVEL_DEBUG, "About to call kh_value");
				kh_value(hashtable, i) = market;
				logit(LOGLEVEL_DEBUG, "kh_value call complete");
				// mark key for deletion on disposal of hashtable
				struct KeyDelete* latest_key = (struct KeyDelete*)malloc(sizeof(struct KeyDelete));
				latest_key->key = key;
				latest_key->next = NULL;
				if (first_key == NULL) {
					first_key = latest_key;
					current_key = latest_key;
				} else {
					current_key->next = latest_key;
					current_key = latest_key;
				}
			} else {
				free(key);
			}
			current_market = current_market->next;
		}
		current = current->next;
	}

	// iterate the hashtable to build Market linked list
	struct Market* head = NULL;
	struct Market* last = NULL;
	for(khiter_t i = kh_begin(hashtable); i != kh_end(hashtable); i++) {
		if (kh_exist(hashtable, i)) {
			struct Market* current_market = kh_value(hashtable, i);
			if (head == NULL) {
				head = current_market;
			} else {
				last->next = current_market;
			}
			last = current_market;
		}
	}
	// destroy keys
	current_key = first_key;
	while (current_key != NULL) {
		if (current_key->key != NULL)
			free(current_key->key);
		first_key = current_key->next;
		free(current_key);
		current_key = first_key;
	}
	// destroy hashtable
	kh_destroy(khash_market, hashtable);
	// return results
	return head;
}


/**
 * helper method to set the name on the vendor struct
 */
void vendor_set_name(struct Vendor* vendor, const char* name) {
	vendor->Name = malloc(strlen(name) + 1);
	strcpy(vendor->Name, name);
}

struct Vendor* vendor_new() {
	struct Vendor* vendor = malloc(sizeof(struct Vendor));
	if (vendor == NULL)
		return NULL;
	//pthread_mutex_init(&vendor->market_mutex, NULL);
	vendor->Name = NULL;
	vendor->current_market = NULL;
	vendor->running = 0;
	vendor->IsInitialized = 0;
	return vendor;
}

void vendor_free(struct Vendor* vendor) {
	//shutdown thread
	vendor->running = 0;
#ifndef SINGLE_THREADED
	void* status;
	pthread_join(vendor->scheduler, &status);
#endif
	// free memory allocations
	free(vendor->Name);
	market_free(vendor->current_market);
	//pthread_mutex_destroy(&vendor->market_mutex);
	free(vendor);
}

void* vendor_update_loop(void* args) {
	int market_timeout = 1800; // 3 minutes
	struct Vendor* vendor = (struct Vendor*)args;
	struct Market* to_be_deleted = NULL;
	do {
		// update markets, keeping the old one around for a bit (market_timeout)
		if (to_be_deleted != NULL) {
			market_free(to_be_deleted);
		}
		to_be_deleted = vendor->current_market;
		vendor->current_market = vendor->markets_get();
		vendor->IsInitialized = 1;
		// pause in 2 loops, check every second, then exit loop after timeout
		int sTotal = 0;
		while(vendor->running && sTotal < market_timeout) {
			sleep(1);
			sTotal++;
		}
	} while (vendor->running);
#ifndef SINGLE_THREADED
	pthread_exit(NULL);
#endif
	return NULL;
}


/***
 * Get the Book struct for a specific vendor
 * @param vendor the name of the vendor
 * @returns the struct Book for that vendor, or NULL on error
 */
struct Vendor* vendor_get(const char* vendor_name) {
	struct Vendor* vendor = vendor_new();
	if (vendor == NULL)
		return NULL;

	vendor_set_name(vendor, vendor_name);

	if (strcmp(vendor_name, "mock") == 0) {
		vendor->books_get = mock_get_books;
		vendor->markets_get = mock_get_markets;
	}
	if (strcmp(vendor_name, "bittrex") == 0) {
		vendor->books_get = bittrex_get_books;
		vendor->markets_get = bittrex_get_markets;
		vendor->limit_buy = bittrex_limit_buy;
		vendor->limit_sell = bittrex_limit_sell;
		vendor->market_buy = bittrex_market_buy;
		vendor->market_sell = bittrex_market_sell;
		vendor->balance = bittrex_balance;
	}
	if (strcmp(vendor_name, "btc38") == 0) {
		vendor->books_get = btc38_get_books;
		vendor->markets_get = btc38_get_markets;
		vendor->limit_buy = btc38_limit_buy;
		vendor->limit_sell = btc38_limit_sell;
		vendor->market_buy = btc38_market_buy;
		vendor->market_sell = btc38_market_sell;
		vendor->balance = btc38_balance;
	}
	if (strcmp(vendor_name, "poloniex")==0) {
		vendor->books_get = poloniex_get_books;
		vendor->markets_get = poloniex_get_markets;
		vendor->limit_buy = poloniex_limit_buy;
		vendor->limit_sell = poloniex_limit_sell;
		vendor->market_buy = poloniex_market_buy;
		vendor->market_sell = poloniex_market_sell;
		vendor->balance = poloniex_balance;
	}
#ifndef SINGLE_THREADED
	vendor->running = 1;
	pthread_create(&(vendor->scheduler), NULL, vendor_update_loop, vendor);
#else
	vendor_update_loop(vendor);
#endif
	return vendor;
}

/**
 * Create a new struct VendorList
 * @returns a pointer to the newly allocated struct VendorList
 */
struct VendorList* vendor_list_new() {
	struct VendorList* list = malloc(sizeof(struct VendorList));
	if (list != NULL) {
		list->next = NULL;
	}
	return list;
}

/***
 * Free allocation of a struct VendorList
 * @param head the first in the list to be freed
 * @param free_vendor true(1) if you want to deallocate the vendor too
 */
void vendor_list_free(struct VendorList* head, int free_vendor) {
	struct VendorList* current = head;
	struct VendorList* next = NULL;
	while (current != NULL) {
		next = current->next;
		if (free_vendor)
			vendor_free(current->vendor);
		free(current);
		current = next;
	}
}

struct VendorList* vendors_get_all() {
	struct VendorList* vendors = vendor_list_new();
	vendors->vendor = vendor_get("btc38");
	vendors->next = vendor_list_new();
	vendors->next->vendor = vendor_get("bittrex");
	vendors->next->next = vendor_list_new();
	vendors->next->next->vendor = vendor_get("poloniex");
	return vendors;
}


struct VendorList* vendors_with_market(struct VendorList* head, const char* base_currency, const char* market_currency) {
	struct VendorList* new_list = NULL;
	struct VendorList* last_added = NULL;
	struct VendorList* current = head;
	while (current != NULL) {
		if (current->vendor->IsInitialized) { // ignore vendors that have yet to be initialized
			// try it the normal way
			const struct Market* mkt = market_get(current->vendor->current_market, base_currency, market_currency);
			if (mkt == NULL) // try it the opposite way
				mkt = market_get(current->vendor->current_market, market_currency, base_currency);
			if (mkt != NULL) {
				if (new_list == NULL) {
					new_list = vendor_list_new();
					last_added = new_list;
				} else {
					last_added->next = vendor_list_new();
					last_added = last_added->next;
				}
				last_added->vendor = current->vendor;
			}
		}
		current = current->next;
	}
	return new_list;
}


