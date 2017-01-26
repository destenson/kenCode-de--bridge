#pragma once

#include "book.h"

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

/**
 * given the VendorList, compile a list of trading pairs available, removing duplicates
 * @param vendor_head the linked list
 * @returns a struct Market linked list of all trading pairs, with only the currency names and "fee"
 */
struct Market* vendor_get_all_trading_pairs(struct VendorList* vendor_head);
