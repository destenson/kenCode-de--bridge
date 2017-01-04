#include <stdio.h>
#include <string.h>

#include "bridge/book.h"
#include "bridge/book_mock.h"

/***
 * Get the Book struct for a specific vendor
 * @param vendor the name of the vendor
 * @returns the struct Book for that vendor, or NULL on error
 */
struct Book* book_for_vendor(const char* vendor) {
	if (strcmp(vendor, "mock") == 0)
		return book_mock_get();
	return NULL;
}
