#include <stdlib.h>
#include <string.h>

#include "bridge/book.h"

/***
 * Get the Book struct for a specific vendor
 * @param vendor the name of the vendor
 * @returns the struct Book for that vendor, or NULL on error
 */
struct Book* book_for_vendor(const char* vendor) {
	struct Book* book = malloc(sizeof(struct Book));
	if (book == NULL)
		return NULL;

	if (strcmp(vendor, "mock") == 0) {
		book->quote = book_mock_quote;
	}
	if (strcmp(vendor, "bittrex") == 0) {
		book->quote = book_bittrex_quote;
	}
	return book;
}
