#include "../include/bridge/book.h"

int test_book_mock() {
	struct Book* mock = book_for_vendor("mock");

	if (mock->quote(SIDE_BIDS, 2) > 0)
		return 0;

	return 1;
}
