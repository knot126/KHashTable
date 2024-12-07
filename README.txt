Knot's Blob-to-Blob Hash Table
------------------------------

This single-header library implements a basic hash table which can map
arbitrary blobs of data to other arbitrary blobs of data. 'Blob' basically
means strings including NUL's.

The hash table has the following properties:

  - Uses the DJB2 hash function
  - Collision resolution using open addressing
  - Preserves the order of keys by insertion by storing indexes to values in
    slots instead of the values themselves
  - Common case O(1) insert, update, retrieve, and member check
  - O(n) delete (makes deleting keys less complex with order-preserving)
  - Only supports power-of-two capacity sizes ATM

Some general usage notes:

  - Exposed hash table functions never make internal copies of KH_Blob's, but
    they will always free them if they aren't used longer term. They "steal"
    them, per se. This applies even to functions like KH_DictHas().
  - In functions where KH_Blob's are returned, copies are also NOT made. You
    should not mutate them.

Short example:

  void func(void) {
      KH_Dict *myDict = KH_CreateDict();
      
      if (!myDict) {
          // Error! Handle it.
      }
      
      // Create some values. We could check if they succeede by checking that
      // their return value is true, but we don't have to.
      KH_DictSet(myDict, KH_BlobForString("roughness"), KH_BlobForString("13"));
      KH_DictSet(myDict, KH_BlobForString("shine"), KH_BlobForString("0.6"));
      KH_DictSet(myDict, KH_BlobForString("shadow"), KH_BlobForString("true"));
      
      printf("shine = %s\n", KH_DictGet(myDict, KH_BlobForString("shine"))->data);
  }

Using blobs:

  - In KHashTable, keys and values are stored in blobs, which are basically
    any buffer. They can contain NUL bytes.
  
  - Almost all functions take "ownership" of the blob you pass them, and use
    it internally or automatically free if it it's not used. If you want to
    pass a blob with the same value as a previous one, you will need to make
    a copy of it.
  
  - KH_Blob *KH_CreateBlob(uint8_t *buffer, size_t length)
    
    Creates a new blob for use with hash table functions. Returns NULL if it
    fails to allocate memory.
  
  - KH_Blob *KH_BlobForString(char *string)
    
    Creates a new blob from a NUL-terminated C string. Returns NULL if it
    fails to allocate memory.
  
  - void KH_ReleaseBlob(KH_Blob *blob)
    
    Releases all resources associated with a blob. Normally, calling this
    function is unnessicary, since the hash table functions free blobs if
    they are not used later.
  
  - The blob structure has the following members:
    
    - data, an array of the bytes the blob contains
    - length, the length of the data the blob holds
    
    Acessing the blob structure directly is needed, since there are no
    functions to get or set data.

Using the hash table (dictionary):

  - KHashTable calls hash tables "dictionaries".
  
  - KH_Dict *KH_CreateDict(void)
    
    Creates a new dictionary. Returns NULL if it fails.
  
  - bool KH_DictSet(KH_Dict *dict, KH_Blob *key, KH_Blob *value)
    
    Sets the value associated with the key to the given value. Returns true
    on success, and false on failure.
  
  - KH_Blob *KH_DictGet(KH_Dict *dict, KH_Blob *key)
    
    Gets the blob assocaited with the given key. The blob returned should be
    treated as immutable and should not be freed, since it's the same one
    used to store the blob internally. Returns NULL if there isn't a value
    assocaited with the given key.
  
  - bool KH_DictHas(KH_Dict *dict, KH_Blob *key)
    
    Check if there is any value assocaited with the given key. Returns true
    if there is, or false if not.
  
  - bool KH_DictDelete(KH_Dict *dict, KH_Blob *key)
    
    Delete the mapping assocaited with the given key. Returns true if
    successful, or false if not.
  
  - void KH_ReleaseDict(KH_Dict *dict)
    
    Releases all resources associated with a dictionary.

Other hash table functions:

  - size_t KH_DictLen(KH_Dict *dict)
    
    Return the number of entries in the hash table.
  
  - KH_Blob *KH_DictKeyIter(KH_Dict *dict, size_t index)
    
    Return the key for the i-th key-value pair in the dictionary, or NULL
    if it would be out of bounds.
  
  - KH_Blob *KH_DictValueIter(KH_Dict *dict, size_t index)
    
    Return the value for the i-th key-value pair in the dictionary, or NULL
    if it would be out of bounds.
