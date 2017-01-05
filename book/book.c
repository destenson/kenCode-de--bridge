#include <stdlib.h>
#include <string.h>

#include "bridge/book.h"

/***
 * Get the Book struct for a specific vendor
 * @param vendor the name of the vendor
 * @returns the struct Book for that vendor, or NULL on error
 */
struct Vendor* vendor_get(const char* vendor_name) {
	struct Vendor* vendor = malloc(sizeof(struct Vendor));
	if (vendor == NULL)
		return NULL;

	if (strcmp(vendor_name, "mock") == 0) {
		vendor->books_get = mock_get_books;
		vendor->markets_get = mock_get_markets;
	}
	if (strcmp(vendor_name, "bittrex") == 0) {
		vendor->books_get = bittrex_get_books;
		vendor->markets_get = bittrex_get_markets;
	}
	return vendor;
}

/**
 * Free memory resources used by linked list of Book
 * @param book the parent node of the book list
 */
void book_free(struct Book* book) {
	if (book != NULL) {
		struct Book* current = book;
		struct Book* next = book->next;
		while (current != NULL) {
			free(current);
			current = next;
			if (current != NULL)
				next = current->next;
		}
	}
}

struct Market* market_new() {
	struct Market* market = (struct Market*)malloc(sizeof(struct Market));
	if (market == NULL)
		return NULL;
	market->base_currency = NULL;
	market->market_currency = NULL;
	market->market_name = NULL;
	market->next = NULL;
	market->min_trade_size = 0.0;
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
