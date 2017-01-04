#include <stdlib.h>

#include "bridge/book.h"

double book_mock_quote(enum Side side, const char* currency_from, const char* currency_to, double qty) {
	if (side == SIDE_BIDS)
		return -2.0;
	if (side == SIDE_ASKS)
		return -1.0;
	return -3.0;
}
