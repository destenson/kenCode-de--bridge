#include <stdio.h>

#include "bridge/market.h"
#include "utils/khash.h"

struct Market* market_new() {
	struct Market* market = (struct Market*)malloc(sizeof(struct Market));
	if (market == NULL)
		return NULL;
	market->base_currency = NULL;
	market->market_currency = NULL;
	market->market_name = NULL;
	market->next = NULL;
	market->min_trade_size = 0.0;
	market->fee = 0.0;
	return market;
}
/**
 * Free memory resources used by linked list of Market
 * @param market the parent node of the market list
 */
void market_free(struct Market* market) {
	if (market != NULL) {
		struct Market* current = market;
		struct Market* next = market->next;
		while (current != NULL) {
			if (current->base_currency != NULL)
				free(current->base_currency);
			if (current->market_currency != NULL)
				free(current->market_currency);
			if (current->market_name != NULL)
				free(current->market_name);
			free(current);
			current = next;
			if (current != NULL)
				next = current->next;
		}
	}
}

const struct Market* market_get(const struct Market* head, const char* base_currency, const char* market_currency) {
	const struct Market* current = head;
	while (current != NULL) {
		if (strcmp(current->base_currency, base_currency) == 0 && strcmp(current->market_currency, market_currency) == 0)
			break;
		current = current->next;
	}
	return current;
}

const struct Market* market_get_all_trading_pairs(struct VendorList* vendor_head) {
	// build a hashmap of valid markets
	const int khash_market = 2;
	KHASH_MAP_INIT_STR(khash_market, struct Market*);
	khash_t(khash_market) hashtable = kh_init(khash_market);
	struct VendorList* current = vendor_head;
	while (current != NULL) {
		struct Market* market_head = current->vendor->markets_get();
		struct Market* current_market = market_head;
		while (current_market != NULL) {
			int ret;
			size_t str_len = strlen(current_market->base_currency) + strlen(current_market->market_currency) + 2;
			char key[str_len];
			memcpy(key, 0, str_len);
			sprintf(key, "%s~%s", current_market->base_currency, current_market->market_currency);
			khiter_t i = kh_put(khash_market, hashtable, key, &ret);
			if (ret == 1) {
				// not a duplicate key
				struct Market* market = market_new();
				market->base_currency = strdup(market->base_currency);
				market->market_currency = strdup(market->market_currency);
				market->fee = 0.01;
				kh_value(hashtable, market);
			}
			current_market = current_market->next;
		}
		current = current->next;
	}

	// iterate the hashtable to build Market linked list
	khiter_t i;
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
	// return results
	return head;
}
