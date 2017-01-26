#include <stdlib.h>
#include <string.h>


#include "bridge/book.h"

/***
 * interface to the different vendors' books
 */



struct Book* book_new() {
	struct Book* book = (struct Book*)malloc(sizeof(struct Book));
	if (book != NULL) {
		book->ask_price = 0.0;
		book->ask_qty = 0.0;
		book->bid_price = 0.0;
		book->bid_qty = 0.0;
		book->next = NULL;
	}
	return book;
}

/**
 * Free memory resources used by linked list of Book
 * @param book the parent node of the book list
 */
void book_free(struct Book* book) {
	if (book != NULL) {
		struct Book* current = book;
		struct Book* next = book->next;
		while (current != NULL) {
			free(current);
			current = next;
			if (current != NULL)
				next = current->next;
		}
	}
}

struct Balance* balance_new() {
	struct Balance* balance = (struct Balance*)malloc(sizeof(struct Balance));
	if (balance == NULL)
		return NULL;
	balance->available = 0.0;
	balance->balance = 0.0;
	balance->currency = NULL;
	balance->pending = 0.0;
	return balance;
}

void balance_free(struct Balance* balance) {
	if (balance != NULL) {
		if (balance->currency != NULL)
			free(balance->currency);
		free(balance);
	}
}
