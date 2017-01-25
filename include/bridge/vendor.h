#pragma once

#include "book.h"
#include "market.h"

/**
 * A collection of function pointers to be used as the interface
 * for a vendor
 */
struct Vendor {
	char* Name;
	int IsInitialized;
	struct Book* (*books_get)(const struct Market* market);
	struct Market* (*markets_get)();
	int (*limit_buy)(const struct Market* currencyPair, double rate, double quantity);
	int (*limit_sell)(const struct Market* currencyPair, double rate, double quantity);
	int (*market_buy)(const struct Market* currencyPair, double quantity);
	int (*market_sell)(const struct Market* currencyPair, double quantity);
	struct Balance* (*balance)(const char* currency);

	// "private" variables
	pthread_t scheduler;
	struct Market* current_market;
	int running;
	//pthread_mutex_t market_mutex;
};

struct VendorList {
	struct Vendor* vendor;
	struct VendorList* next;
};

struct Vendor* vendor_new();
void vendor_free(struct Vendor* vendor);

/**
 * Retrieves all known vendors
 * @returns a struct VendorList that is a linked list to each vendor
 */
struct VendorList* vendors_get_all();

/***
 * Shut down all vendors in list and deallocate memory
 * @param head the list of vendors
 * @param free_vendor true(1) if vendor object should be freed too
 */
void vendor_list_free(struct VendorList* head, int free_vendor);

struct VendorList* vendors_with_market(struct VendorList* head, const char* base_currency, const char* market_currency);

/***
 * Get the function pointers for a specific vendor
 * @param vendor the name of the vendor
 * @returns the struct of function pointers for the vendor
 */
struct Vendor* vendor_get(const char* vendor_name);
