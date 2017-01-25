#include "khash.h"

const int khStrStr = 32;
KHASH_MAP_INIT_STR(khStrStr, char*);

struct HashTable {
	khash_t(khStrStr)* khash_table;
};

struct HashTable* hashtable_new() {
	struct HashTable* ht = (struct HashTable*)malloc(sizeof(struct HashTable));
	ht->khash_table = kh_init(khStrStr);
	return ht;
}

void hashtable_free(struct HashTable* ht) {
	kh_destroy(khStrStr, ht->khash_table);
}

void hashtable_put(struct HashTable* ht, char* key, char* value) {
	int ret;
	khiter_t k = kh_put(khStrStr, ht->khash_table, key, &ret);
	kh_value(ht->khash_table, k) = value;
}

char* hashtable_get(struct HashTable* ht, char* key) {
	khiter_t k = kh_get(khStrStr, ht->khash_table, key);
	if (k == kh_end(ht->khash_table))
		return NULL;
	return kh_val(ht->khash_table, k);
}

