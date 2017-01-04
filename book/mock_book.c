#include <stdlib.h>

#include "bridge/book.h"
#include "bridge/book_mock.h"

double price(enum Side side, double qty) {
	if (side == SIDE_BIDS)
		return -2.0;
	if (side == SIDE_ASKS)
		return -1.0;
	return -3.0;
}

struct Book* book_mock_get() {
	struct Book* retVal = (struct Book*) malloc(sizeof(struct Book));
	retVal->quote = price;
	return retVal;
}
