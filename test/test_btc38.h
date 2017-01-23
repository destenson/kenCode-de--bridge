#include <stdio.h>

#include "bridge/book.h"

int test_btc38_account_balance() {
	int retVal = 0;
	const char* method = "test_btc38_account_balance";

	struct Vendor* vendor = vendor_get("btc38");
	struct Balance* balance = vendor->balance("BTC");
	if (balance == NULL) {
		fprintf(stderr, "%s: Unable to get balance\n", method);
		goto exit;
	}
	retVal = 1;
	exit:
	balance_free(balance);
	vendor_free(vendor);
	return retVal;
}
