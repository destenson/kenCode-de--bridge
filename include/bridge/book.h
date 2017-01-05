/***
 * An interface for the connection to different books
 * to retrieve the book from them
 */
enum Side { SIDE_BIDS, SIDE_ASKS };

struct Book {
	double bid_qty;
	double bid_price;
	double ask_qty;
	double ask_price;
	struct Book* next;
};

struct Market {
	char* market_name;
	char* market_currency;
	char* base_currency;
	double min_trade_size;
	struct Market* next;
};

struct Vendor {
	struct Book* (*books_get)(const struct Market* market);
	struct Market* (*markets_get)();
};

/***
 * Get the Book struct for a specific vendor
 * @param vendor the name of the vendor
 * @returns the struct Book for that vendor, or NULL on error
 */
struct Vendor* vendor_get(const char* vendor_name);

const struct Market* market_get(const struct Market* head, const char* base_currency, const char* market_currency);

struct Market* market_new();
void market_free(struct Market* market);
void book_free(struct Book* book);

// vendors
// bittrex
struct Book* bittrex_get_books(const struct Market* market);
struct Market* bittrex_get_markets();
// mock
struct Book* mock_get_books(const struct Market* market);
struct Market* mock_get_markets();
