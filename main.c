#include <stdio.h>

#define KHASHTABLE_IMPLEMENTATION
#include "hashtable.h"

void print_dict(KH_Dict *dict) {
	size_t size = KH_DictLen(dict);
	printf("dict contents (%zu items):\n", size);
	
	for (size_t i = 0; i < size; i++) {
		printf("\t%s -> %s\n", KH_DictKeyIter(dict, i)->data, KH_DictValueIter(dict, i)->data);
	}
}

int main(int argc, char *argv[]) {
	KH_Dict *myDict = KH_CreateDict();
	
	KH_DictSet(myDict, KH_BlobForString("hello"), KH_BlobForString("world!"));
	KH_DictSet(myDict, KH_BlobForString("balls"), KH_BlobForString("I have 69 balls!"));
	KH_DictSet(myDict, KH_BlobForString("iscute"), KH_BlobForString("no :<"));
	KH_DictSet(myDict, KH_BlobForString("urmom"), KH_BlobForString("hehehe"));
	
	KH_DictSet(myDict, KH_BlobForString("coffee"), KH_BlobForString("stain"));
	KH_DictSet(myDict, KH_BlobForString("shitstain"), KH_BlobForString("coffeestain"));
	KH_DictSet(myDict, KH_BlobForString("knot"), KH_BlobForString("one two six"));
	KH_DictSet(myDict, KH_BlobForString(":3"), KH_BlobForString("UwU"));
	
	KH_DictSet(myDict, KH_BlobForString("poop"), KH_BlobForString("name of my cat!"));
	KH_DictSet(myDict, KH_BlobForString("skibidi"), KH_BlobForString("L rizz"));
	
	KH_DictDelete(myDict, KH_BlobForString("skibidi"));
	
	printf("Value for %s: %s\n", ":3", KH_DictGet(myDict, KH_BlobForString(":3"))->data);
	
	print_dict(myDict);
	
	KH_ReleaseDict(myDict);
	
	return 0;
}
