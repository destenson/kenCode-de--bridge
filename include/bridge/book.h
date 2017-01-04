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
 * @param qty the quantity to ask for
 * @returns the price of the coins, or negative number on error
 */
double (*quote)(enum Side side, double qty);

};

/***
 * Get the Book struct for a specific vendor
 * @param vendor the name of the vendor
 * @returns the struct Book for that vendor, or NULL on error
 */
struct Book* book_for_vendor(const char* vendor);
