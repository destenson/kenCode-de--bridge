#include <string.h>
#include <stdio.h>

#include "test_book.h"
#include "test_ws.h"

int testit(const char* name, int (*func)(void)) {
	printf("Testing %s...\n", name);
	int retVal = func();
	if (retVal)
		printf("%s success!\n", name);
	else
		printf("** Uh oh! %s failed.**\n", name);
	return retVal == 0;
}

const char* names[] = {
		"test_book_mock",
		"test_book_bittrex_url",
		"test_book_bittrex",
		"test_market_bittrex",
		"test_book_btc38",
		"test_book_all",
		"test_book_poloniex"
};

int (*funcs[])(void) = {
		test_book_mock,
		test_book_bittrex_url,
		test_book_bittrex,
		test_market_bittrex,
		test_book_btc38,
		test_book_all,
		test_book_poloniex
};

/**
 * run 1 test or run all
 */
int main(int argc, char** argv) {
	int counter = 0;
	int tests_ran = 0;
	char* test_wanted;
	int only_one = 0;
	if(argc > 1) {
		only_one = 1;
		if (argv[1][0] == '\'') { // some shells put quotes around arguments
			argv[1][strlen(argv[1])-1] = 0;
			test_wanted = &(argv[1][1]);
		}
		else
			test_wanted = argv[1];
	}
	int array_length = sizeof(funcs) / sizeof(funcs[0]);
	int array2_length = sizeof(names) / sizeof(names[0]);
	if (array_length != array2_length) {
		printf("Test arrays are not of the same length. Funcs: %d, Names: %d\n", array_length, array2_length);
	}
	for (int i = 0; i < array_length; i++) {
		if (only_one) {
			const char* currName = names[i];
			if (strcmp(currName, test_wanted) == 0) {
				tests_ran++;
				counter += testit(names[i], funcs[i]);
			}
		}
		else
			if (!only_one) {
				tests_ran++;
				counter += testit(names[i], funcs[i]);
			}
	}

	if (tests_ran == 0)
		printf("***** No tests found *****\n");
	else {
		if (counter > 0) {
			printf("***** There were %d failed test(s) *****\n", counter);
		} else {
			printf("All %d tests passed\n", tests_ran);
		}
	}
	return 1;
}
