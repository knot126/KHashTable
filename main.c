#define KHASHTABLE_IMPLEMENTATION
#include "hashtable.h"
#include <stdio.h>

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
	
	print_dict(myDict);
	
	KH_ReleaseDict(myDict);
	
	return 0;
}
