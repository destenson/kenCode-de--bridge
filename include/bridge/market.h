#pragma once

#include "bridge/book.h"

/***
 * Provides a comfortable estimate of the size of the buffer needed to encode this market
 * @param market what is to be encoded. NOTE: this method does not traverse the list
 * @returns number of bytes estimated to encode this struct Market
 */
size_t market_protobuf_encode_size(struct Market* market);

/**
 * Provides a comfortable estimate of the size of the buffer needed to encode the linked list
 * @param the first element to be encoded. NOTE: this will traverse the linked list
 * @returns number of bytes estimated to encode this linked list
 */
size_t market_list_protobuf_encode_size(struct Market* head);

/***
 * Encode this struct Market
 * @param market the struct Market to be encoded. NOTE: does not traverse the list
 * @param buffer where to put the data
 * @param max_buffer_length the amount of space reserved for buffer
 * @param bytes_written the number of bytes written into the buffer
 * @returns true(1) on success
 */
int market_protobuf_encode(struct Market* market, unsigned char* buffer, size_t max_buffer_length, size_t* bytes_written);

/***
 * Encode this struct Market linked list
 * @param head the first Market to be encoded
 * @param buffer where to put the results
 * @param max_buffer_length the maximum size of the buffer
 * @param bytes_written how many bytes were written into the buffer
 * @returns true(1) on success
 */
int market_list_protobuf_encode(struct Market* head, unsigned char* buffer, size_t max_buffer_length, size_t* bytes_written);

/**
 * Create a new struct Market "object"
 */
struct Market* market_new();

/**
 * Free memory resources used by linked list of Market
 * @param market the parent node of the market list
 */
void market_free(struct Market* market);

/***
 * Given the base and market currency, get the appropriate market from the linked list
 * @param head the first of the linked list
 * @param base_currency the base currency that we're searching for
 * @param market_currency the market currency that we're searching for
 * @returns the mathing struct Market item, or NULL if none found
 */
const struct Market* market_get(const struct Market* head, const char* base_currency, const char* market_currency);

