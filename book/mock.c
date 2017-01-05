#include <stdlib.h>
#include <string.h>

#include "bridge/book.h"

double book_mock_quote(enum Side side, const char* currency_from, const char* currency_to, double qty) {
	if (side == SIDE_BIDS)
		return -2.0;
	if (side == SIDE_ASKS)
		return -1.0;
	return -3.0;
}

struct Market* mock_get_markets() {
	struct Market* market = market_new();
	market->base_currency = malloc(4);
	strcpy(market->base_currency, "BTC");
	market->market_currency = malloc(4);
	strcpy(market->market_currency, "LTC");
	market->min_trade_size = 0.01;
	return market;
}

struct Book* mock_get_books(const struct Market* market) {
	struct Book* book = (struct Book*) malloc(sizeof(struct Book));
	book->next = NULL;
	book->bid_qty = 12;
	book->bid_price = 0.001;
	book->ask_qty = 12;
	book->ask_price = 0.01;
	return book;
}
