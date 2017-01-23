#include <stdio.h>

#include "bridge/book.h"

int test_bittrex_account_balance() {
	int retVal = 0;
	const char* method = "test_bittrex_account_balance";

	struct Vendor* vendor = vendor_get("bittrex");
	struct Balance* balance = vendor->balance("BTC");
	if (balance == NULL) {
		fprintf(stderr, "%s: Unable to get balance\n", method);
		goto exit;
	}
	retVal = 1;
	exit:
	return retVal;
}
