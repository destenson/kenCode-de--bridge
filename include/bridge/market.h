#pragma once

#include "book.h"
#include "vendor.h"

/**
 * A market that a vendor has
 */
struct Market {
	char* market_name;
	char* market_currency;
	char* base_currency;
	double min_trade_size;
	double fee;
	struct Market* next;
};

/**
 * Given the list of vendors, get all markets available. NOTE: Many areas of the struct are not set. Only the two currencies and the fee
 * @param vendor_head a linked list of vendors
 * @returns a Market linked list of all pairs available from the list of vendors, with duplicates removed
 */
struct Market* market_get_all_trading_pairs(struct VendorList* vendor_head);

