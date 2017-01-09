#include <pthread.h>

/***
 * An interface for the connection to different books from different vendors
 */

/**
 * A bid and ask entry in a vendor's market's book
 */
struct Book {
	double bid_qty;
	double bid_price;
	double ask_qty;
	double ask_price;
	struct Book* next;
};

/**
 * A market that a vendor has
 */
struct Market {
	char* market_name;
	char* market_currency;
	char* base_currency;
	double min_trade_size;
	struct Market* next;
};

/**
 * A collection of function pointers to be used as the interface
 * for a vendor
 */
struct Vendor {
	char* Name;
	int IsInitialized;
	struct Book* (*books_get)(const struct Market* market);
	struct Market* (*markets_get)();
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
 * Search the linked list for a specific market
 * @param head the start of the linked list to search
 * @param base_currency the base currency
 * @param market_currency the market currency
 * @returns the struct Market that matches, or NULL if not found
 */
const struct Market* market_get(const struct Market* head, const char* base_currency, const char* market_currency);

/**
 * Allocates memory for a struct Market
 * @returns a pointer to a memory-allocated struct Market
 */
struct Market* market_new();

/**
 * Free the memory allocated by market_new
 * @param market the struct Market to free
 */
void market_free(struct Market* market);

/***
 * Allocates the memory for a struct Book
 * @returns a pointer to a memory-allocated struct Book
 */
struct Book* book_new();

/**
 * Free the memory allocated by book_new
 * @param book the struct Book to free
 */
void book_free(struct Book* book);

// vendors
// mock (for testing)
struct Book* mock_get_books(const struct Market* market);
struct Market* mock_get_markets();
// bittrex
struct Book* bittrex_get_books(const struct Market* market);
struct Market* bittrex_get_markets();
//btc38
struct Book* btc38_get_books(const struct Market* market);
struct Market* btc38_get_markets();
//poloniex
struct Book* poloniex_get_books(const struct Market* market);
struct Market* poloniex_get_markets();
