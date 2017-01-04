/***
 * An interface for the connection to different books
 * to retrieve the book from them
 */
enum Side { SIDE_BIDS, SIDE_ASKS };

struct Book {

/***
 * Retrieve the price for (qty) coins on
 * either the bid or ask
 * @param side bid or ask
 * @param currency_from the currency we wish to exchange
 * @param currency_to the currency we wish to receive
 * @param qty the quantity to ask for
 * @returns the price of [qty] of [currency_to], or negative number on error
 */
double (*quote)(enum Side side, const char* currency_from, const char* currency_to, double qty);

};

/***
 * Get the Book struct for a specific vendor
 * @param vendor the name of the vendor
 * @returns the struct Book for that vendor, or NULL on error
 */
struct Book* book_for_vendor(const char* vendor);

// vendors
// bittrex
double book_bittrex_quote(enum Side side, const char* currency_from, const char* currency_to, double qty);
// mock
double book_mock_quote(enum Side side, const char* currency_from, const char* currency_to, double qty);
