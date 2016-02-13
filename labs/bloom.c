#include "bloom.h"

index_t hash1(bloom_filter_t *B, key_t k) {
	key_t high5 = k & 0xf800000000000000;
	k = k << 5;
	k = k ^ (high5 >> 59);
	return k % (B->size); 
}

index_t hash2(bloom_filter_t *B, key_t k) {
	return k % (B->size);
}

index_t hashN(bloom_filter_t *B, key_t k, index_t n) {
	return (hash1(B, k) + n * hash2(B,k)) % (B->size);
}

void set_bit(bloom_filter_t *B, index_t i) {
	index_t num = B->table[i >> 6];
	index_t b_idx = i % 64;
	index_t shift = 1;
	num = num | (shift << b_idx);
	B->table[i >> 6] = num;
}

index_t get_bit(bloom_filter_t *B, index_t i) {
	index_t num = B->table[i >> 6];
	index_t b_idx = i % 64;
	index_t shift = 1;
	return ((num & (shift << b_idx)) >> b_idx);
}

void bloom_init(bloom_filter_t *B, index_t size_in_bits) {
	index_t i;
	B->size = size_in_bits;
	B->count = 0;
	if (B->size % 64 == 0)
		B->table = (index_t *) malloc(sizeof(index_t)*(B->size)/64);
	else
		B->table = (index_t *) malloc(sizeof(index_t)*(1+(B->size)/64));

	for (i = 0 ; i < sizeof(B->table)/sizeof(index_t); ++i) {
		(B->table)[i] = 0;
	}
}

void bloom_destroy(bloom_filter_t *B) {
	free(B->table);
}

int bloom_check(bloom_filter_t *B, key_t k) {
	int i;
	for (i= 0 ; i < N_HASHES ; ++i) {
		if (i == 0) {
			if (get_bit(B, hash1(B, k)) == 0)
				return 0;
		}
		else if (i == 1) {
			if (get_bit(B, hash2(B, k)) == 0)
				return 0;
		}
		else {
			if (get_bit(B, hashN(B,k,i)) == 0)
				return 0;
		}
	}
	return 1;
}

void bloom_add(bloom_filter_t *B, key_t k) {
	int i;
	for (i= 0 ; i < N_HASHES ; ++i) {
		if (i == 0) {
			set_bit(B, hash1(B, k));
		}
		else if (i == 1) {
			set_bit(B, hash2(B, k));
		}
		else {
			set_bit(B, hashN(B,k,i));
		}
	}
	(B->count)++;
}

int* rand100() {
	int i;
	int* n = (int*) malloc(sizeof(int)*100);
	for (i = 0 ; i < 100 ; ++i)
		n[i] = rand() % 1000000;
	return n;
}

void runTest(int* inputs, int* checks) {
	index_t i;
	index_t size = 1000;
	index_t count = 0;
	bloom_filter_t* b = (bloom_filter_t*) malloc(sizeof(bloom_filter_t));
	bloom_init(b, size);

	for (i = 0 ; i < 100 ; ++i) {
		bloom_add(b, inputs[i]);
	}
	for (i = 0 ; i < size ; ++i) {
		count += get_bit(b, i);
	}
	printf("Smoke: %llu\n", count);
	count = 0;
	for (i = 0 ; i < 100 ; ++i) {
		count += bloom_check(b, checks[i]);
	}
	printf("Collisions: %llu\n", count);

	bloom_destroy(b);
	free(b);
}

int main( int argc, const char* argv[] )
{
	int* inputs = rand100();
	int* checks = rand100();
	runTest(inputs, checks);
	free(inputs);
	free(checks);
	return 0;
}