#ifndef _KH_HEADER
#define _KH_HEADER
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

enum {
	KH_HASH_EMPTY = 0xffffffff,
	KH_HASH_DELETED = 0xfffffffe,
};

typedef uint32_t kh_hash_t;
typedef struct KH_Blob {
	size_t length;
	kh_hash_t hash;
	const uint8_t data[0];
} KH_Blob;

typedef struct KH_Slot {
	uint32_t index;
} KH_Slot;

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

static uint32_t KH_BlobSlotIndexForSz(uint32_t hash, size_t size) {
	// WARNING: Only works for powers of two
	return hash & (size - 1);
}

static void KH_InsertSlot(KH_Slot *slots, size_t nslots, uint32_t hash, uint32_t index) {
	uint32_t slot_index = KH_BlobSlotIndexForSz(hash, nslots);
	
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
		return NULL;
	}
	
	// Init slots to empty (0xFFFFFFFF)
	memset(new_slots, 0xff, sizeof *self->slots * new_size);
	
	// Init pairs to empty (NULL)
	memset(new_pairs, 0, sizeof *self->pairs * new_size);
	
	// Copy old pairs to new pairs
	for (size_t i = 0, j = 0; i < self->data_alloced; i++) {
		if (!self->pairs[i].key || !self->pairs[i].value) {
			continue;
		}
		
		new_pairs[j].key = self->pairs[i].key;
		new_pairs[j].value = self->pairs[i].value;
		j++;
	}
	
	// Copy 
}
#endif // KHASHTABLE_IMPLEMENTATION

#endif // _KH_HEADER
