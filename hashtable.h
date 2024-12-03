#ifndef _KH_HEADER
#define _KH_HEADER
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

enum {
	KH_HASH_EMPTY = 0xffffffff,
	KH_HASH_DELETED = 0xfffffffe,
};

#define KH_NOT_FOUND ((size_t)-1)

typedef uint32_t kh_hash_t;
typedef struct KH_Blob {
	size_t length;
	kh_hash_t hash;
	const uint8_t data[0];
} KH_Blob;

// typedef struct KH_Slot {
// 	uint32_t index;
// } KH_Slot;
typedef struct uint32_t KH_Slot;

typedef struct KH_DictPair {
	KH_Blob *key;
	KH_Blob *value;
} KH_DictPair;

typedef struct KH_Dict {
	KH_Slot *slots;
	KH_DictPair *pairs;
	size_t data_count;
	size_t data_alloced; // Must be a power of two
} KH_Dict;

#ifdef KHASHTABLE_IMPLEMENTATION
static kh_hash_t KH_Hash(const uint8_t *buffer, const size_t length) {
	kh_hash_t hash = 5381;
	
	for (size_t i = 0; i < length; i++) {
		hash = ((hash << 5) + hash) ^ buffer[i];
	}
	
	return hash;
}

KH_Blob *KH_CreateBlob(uint8_t *buffer, size_t length) {
	KH_Blob *blob = malloc(sizeof *blob + length);
	
	if (!blob) {
		return NULL;
	}
	
	blob->length = length;
	blob->hash = KH_Hash(buffer, length);
	memcpy((void *) blob->data, buffer, length);
	
	return blob;
}

static bool KH_BlobEqual(KH_Blob *blob1, KH_Blob *blob2) {
	if (blob1 == blob2) {
		return true;
	}
	
	if (blob1->hash != blob2->hash || blob1->length != blob2->length) {
		return false;
	}
	
	return memcmp(blob1->data, blob2->data, blob1->length) == 0;
}

void KH_ReleaseBlob(KH_Blob *blob) {
	free(blob);
}

static uint32_t KH_BlobStartingIndexForSize(uint32_t hash, size_t size) {
	// WARNING: Only works for powers of two
	return hash & (size - 1);
}

static void KH_InsertSlot(KH_Slot *slots, size_t nslots, uint32_t hash, uint32_t index) {
	uint32_t slot_index = KH_BlobStartingIndexForSize(hash, nslots);
	
	while (1) {
		slot_index = (slot_index + 1) & (nslots - 1);
		
		if (slots[slot_index] == KH_HASH_EMPTY || slots[slot_index] == KH_HASH_DELETED) {
			slots[slot_index] = index;
		}
	}
}

static KH_Dict *KH_ResizeDict(KH_Dict *self) {
	/**
	 * Resize a dict, or if it has size zero, allocate the initial memory.
	 */
	
	// New size of the prealloced memory and index data
	size_t new_size = (self->data_alloced) ? (2 * self->data_alloced) : (8);
	
	// Alloc new slots and pair data
	KH_Slot *new_slots = malloc(sizeof *self->slots * new_size);
	KH_DictPair *new_pairs = malloc(sizeof *self->pairs * new_size);
	
	if (!new_slots || !new_pairs) {
		free(new_slots);
		free(new_pairs);
		return NULL;
	}
	
	// Init slots to empty (0xFFFFFFFF)
	memset(new_slots, 0xff, sizeof *self->slots * new_size);
	
	// Init pairs to empty (NULL)
	memset(new_pairs, 0, sizeof *self->pairs * new_size);
	
	// Copy old pairs to new pairs
	size_t j = 0;
	
	for (size_t i = 0; i < self->data_alloced; i++) {
		if (!self->pairs[i].key || !self->pairs[i].value) {
			continue;
		}
		
		new_pairs[j].key = self->pairs[i].key;
		new_pairs[j].value = self->pairs[i].value;
		KH_InsertSlot(new_slots, new_size, new_pairs[j].key->hash, j);
		j++;
	}
	
	// j is now the new data_count
	
	// We should be ready to free old stuff, place new stuff
	free(self->slots);
	free(self->pairs);
	self->slots = new_slots;
	self->pairs = new_pairs;
	self->data_alloced = new_size;
	self->data_count = j;
	
	return self;
}

static bool KH_DictInsert(KH_Dict *self, KH_Blob *key, KH_Blob *value) {
	/**
	 * Insert an entry into the hash table. The key must not exist.
	 */
	
	// Resize if load factor > 0.625, around Wikipedia's recommendation of
	// resizing at 0.6-0.75
	if (((self->data_count >> 1) + (self->data_count >> 3)) > self->data_alloced) {
		self = KH_ResizeDict(self);
		
		if (!self) {
			return false;
		}
	}
	
	self->pairs[self->data_count].key = key;
	self->pairs[self->data_count].value = value;
	
	KH_InsertSlot(self->slots, self->data_alloced, key->hash, self->data_count);
	
	self->data_count++;
	
	return true;
}

static size_t KH_DictLookupIndex(KH_Dict *self, KH_Blob *key) {
	/**
	 * Find the index of a pair given its key. Returns the index or KH_NOT_FOUND
	 * if none was found.
	 */
	
	uint32_t slot_index = KH_BlobStartingIndexForSize(key->hash, self->data_alloced);
	
	for (size_t i = 0; i < self->data_alloced; i++) {
		KH_Slot slot = self->slots[(slot_index + i) & (self->data_count - 1)];
		
		// Empty, never-used slot which won't have anything we're looking for
		// located after it
		if (slot == KH_HASH_EMPTY) {
			break;
		}
		
		// Once used slot which may still have hits after it
		if (slot == KH_HASH_DELETED) {
			continue;
		}
		
		// If the key we're looking up matches the key indexed by the current
		// slot, this is a hit and it should be returned.
		if (KH_BlobEqual(key, self->pairs[slot].key)) {
			return slot;
		}
	}
	
	return KH_NOT_FOUND;
}

static void KH_DictChange(KH_Dict *self, size_t index, KH_Blob *value) {
	/**
	 * Change the value for a key that already exists, given the index to the
	 * key.
	 */
	
	free(self->pairs[index].value);
	self->pairs[index].value = value;
}

bool KH_DictSet(KH_Dict *self, KH_Blob *key, KH_Blob *value) {
	/**
	 * Insert a (key, value) pair into the dictionary, overwriting any existing
	 * one.
	 */
	
	size_t index = KH_DictLookupIndex(self, key);
	
	if (index == KH_NOT_FOUND) {
		return KH_DictInsert(self, key, value);
	}
	else {
		KH_DictChange(self, index, value);
		free(key);
		return true;
	}
}

KH_Blob *KH_DictGet(KH_Dict *self, KH_Blob *key) {
	/**
	 * Get a value blob by a key
	 */
	
	size_t index = KH_DictLookupIndex(self, key);
	
	if (index == KH_NOT_FOUND) {
		return NULL;
	}
	else {
		return self->pairs[index].value;
	}
}

bool KH_DictHas(KH_Dict *self, KH_Blob *key) {
	/**
	 * Check if the dict has a pair with a given key
	 */
	
	return KH_DictLookupIndex(self, key) != KH_NOT_FOUND;
}
#endif // KHASHTABLE_IMPLEMENTATION

#endif // _KH_HEADER
