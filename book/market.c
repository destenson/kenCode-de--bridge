#include <stdio.h>

#include "bridge/book.h"
#include "utils/khash.h"
#include "protobuf.h"

// the "outer" envelope              repeated copies of Market instances
enum WireType market_list_fields[] = { WIRETYPE_LENGTH_DELIMITED };
 // the "inner" envelope           base currency                 market currency        fee (multiplied by 1k to avoid decimal)
enum WireType market_fields[] = { WIRETYPE_LENGTH_DELIMITED, WIRETYPE_LENGTH_DELIMITED, WIRETYPE_VARINT };


/***
 * Provides a comfortable estimate of the size of the buffer needed to encode this market
 * @param market what is to be encoded. NOTE: this method does not traverse the list
 * @returns number of bytes estimated to encode this struct Market
 */
size_t market_protobuf_encode_size(struct Market* market) {
	size_t retVal = 0;
	retVal += strlen(market->base_currency) + 11;
	retVal += strlen(market->market_currency) + 11;
	retVal += 11; // fee
	return retVal;
}

/**
 * Provides a comfortable estimate of the size of the buffer needed to encode the linked list
 * @param the first element to be encoded. NOTE: this will traverse the linked list
 * @returns number of bytes estimated to encode this linked list
 */
size_t market_list_protobuf_encode_size(struct Market* head) {
	size_t retVal = 0;
	struct Market* current = head;
	while (current != NULL) {
		retVal += market_protobuf_encode_size(current) + 11; // market record + field number & type
		current = current->next;
	}
	return retVal;
}


/***
 * Encode this struct Market
 * @param market the struct Market to be encoded. NOTE: does not traverse the list
 * @param buffer where to put the data
 * @param max_buffer_length the amount of space reserved for buffer
 * @param bytes_written the number of bytes written into the buffer
 * @returns true(1) on success
 */
int market_protobuf_encode(struct Market* market, unsigned char* buffer, size_t max_buffer_length, size_t* bytes_written) {
	// 3 fields, base currency, market currency, and fee
	size_t bytes_used = 0;
	int retVal = 0;
	*bytes_written = 0;
	// encode the "inner" market fields
	retVal = protobuf_encode_string(1, market_fields[0], market->base_currency,
			&buffer[*bytes_written], max_buffer_length - *bytes_written, &bytes_used);
	if (retVal == 0)
		return 0;
	*bytes_written += bytes_used;
	retVal = protobuf_encode_string(2, market_fields[1], market->market_currency,
			&buffer[*bytes_written], max_buffer_length - *bytes_written, &bytes_used);
	if (retVal == 0)
		return 0;
	*bytes_written += bytes_used;
	retVal = protobuf_encode_varint(3, market_fields[2], (int)(market->fee * 1000),
			&buffer[*bytes_written], max_buffer_length - *bytes_written, &bytes_used);
	if (retVal == 0)
		return 0;
	return 1;
}

/***
 * Encode this struct Market linked list
 * @param head the first Market to be encoded
 * @param buffer where to put the results
 * @param max_buffer_length the maximum size of the buffer
 * @param bytes_written how many bytes were written into the buffer
 * @returns true(1) on success
 */
int market_list_protobuf_encode(struct Market* head, unsigned char* buffer, size_t max_buffer_length, size_t* bytes_written) {
	size_t bytes_used = 0;
	int retVal = 0;
	*bytes_written = 0;
	struct Market* current = head;
	while (current != NULL) {
		size_t market_len = market_protobuf_encode_size(current);
		unsigned char market[market_len];
		retVal = market_protobuf_encode(current, market, market_len, &market_len);
		if (retVal == 0)
			return 0;
		retVal = protobuf_encode_length_delimited(1, market_list_fields[0], (char*)market, market_len,
				&buffer[*bytes_written], max_buffer_length - *bytes_written, &bytes_used);
		if (retVal == 0)
			return 0;
		*bytes_written += bytes_used;
		current = current->next;
	}
	return 1;
}

struct Market* market_new() {
	struct Market* market = (struct Market*)malloc(sizeof(struct Market));
	if (market == NULL)
		return NULL;
	market->base_currency = NULL;
	market->market_currency = NULL;
	market->market_name = NULL;
	market->next = NULL;
	market->min_trade_size = 0.0;
	market->fee = 0.0;
	return market;
}
/**
 * Free memory resources used by linked list of Market
 * @param market the parent node of the market list
 */
void market_free(struct Market* market) {
	if (market != NULL) {
		struct Market* current = market;
		struct Market* next = market->next;
		while (current != NULL) {
			if (current->base_currency != NULL)
				free(current->base_currency);
			if (current->market_currency != NULL)
				free(current->market_currency);
			if (current->market_name != NULL)
				free(current->market_name);
			free(current);
			current = next;
			if (current != NULL)
				next = current->next;
		}
	}
}

const struct Market* market_get(const struct Market* head, const char* base_currency, const char* market_currency) {
	const struct Market* current = head;
	while (current != NULL) {
		if (strcmp(current->base_currency, base_currency) == 0 && strcmp(current->market_currency, market_currency) == 0)
			break;
		current = current->next;
	}
	return current;
}

