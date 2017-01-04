/***
 * utilities for https
 */

/***
 * Send an HTTPS GET and receive the results
 * @param url the complete url (i.e. 'https://myserver.org/market/getmarket?pair=BTCUSD&nonce=....')
 * @param results where to store the results. NOTE: memory will be allocated
 * @returns 0 on success, otherwise a negative number that denotes the error
 */
int utils_https_get(const char* url, char** results);
