// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free
// software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed
// in the hope that it will be useful, but with permitted additional restrictions
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT
// distributed with this program. You should have received a copy of the
// GNU General Public License along with permitted additional restrictions
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection
#ifndef DICTIONARY_HEADER
#define DICTIONARY_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

template <class K, class V> class DNode
{
public:
    K key;
    V value;
    DNode<K, V>* hashNext;
};

template <class K, class V> class Dictionary
{
public:
    Dictionary(unsigned (*hashFn)(K& key));
    ~Dictionary();

    void clear(void);
    char add(K& key, V& value);
    char getValue(K& key, V& value);
    void print(FILE* out) const;
    unsigned getSize(void) const;
    unsigned getEntries(void) const;
    char contains(K& key);
    char updateValue(K& key, V& value);
    char remove(K& key, V& value);
    char remove(K& key);
    char removeAny(K& key, V& value);
    char iterate(int& index, int& offset, V& value) const;

private:
    void shrink(void);
    void expand(void);

    DNode<K, V>** table;
    unsigned entries;
    unsigned size;
    unsigned tableBits;
    unsigned log2Size;
    char keepSize;
    unsigned (*hashFunc)(K& key);
    unsigned keyHash(K& key);

    const double SHRINK_THRESHOLD;
    const double EXPAND_THRESHOLD;
    const int MIN_TABLE_SIZE;
};

template <class K, class V>
Dictionary<K, V>::Dictionary(unsigned (*hashFn)(K& key))
    : SHRINK_THRESHOLD(0.20)
    , EXPAND_THRESHOLD(0.80)
    , MIN_TABLE_SIZE(32)
{
    log2Size = MIN_TABLE_SIZE;
    size = MIN_TABLE_SIZE;
    assert(size >= 4);
    tableBits = 0;

    while (log2Size) {
        tableBits++;
        log2Size >>= 1;
    }

    tableBits--;
    size = 1 << tableBits;
    entries = 0;
    keepSize = false;

    table = (DNode<K, V>**)new DNode<K, V>*[size];
    assert(table != NULL);

    memset((void*)table, 0, size * sizeof(void*));
    hashFunc = hashFn;
}

template <class K, class V> Dictionary<K, V>::~Dictionary()
{
    clear();
    delete[] table;
}

template <class K, class V> void Dictionary<K, V>::clear()
{
    DNode<K, V>*temp, *del;
    unsigned i;

    for (i = 0; i < size; i++) {
        temp = table[i];
        while (temp != NULL) {
            del = temp;
            temp = temp->hashNext;
            delete (del);
        }
        table[i] = NULL;
    }
    entries = 0;

    while ((getSize() > (unsigned)MIN_TABLE_SIZE) && (keepSize == false)) {
        shrink();
    }
}

template <class K, class V> unsigned Dictionary<K, V>::keyHash(K& key)
{
    unsigned retval = hashFunc(key);
    retval &= ((1 << tableBits) - 1);
    assert(retval < getSize());
    return retval;
}

template <class K, class V> void Dictionary<K, V>::print(FILE* out) const
{
    DNode<K, V>* temp;
    unsigned i;

    fprintf(out, "--------------------\n");
    for (i = 0; i < getSize(); i++) {
        temp = table[i];

        fprintf(out, " |\n");
        fprintf(out, "[ ]");

        while (temp != NULL) {
            fprintf(out, "--[ ]");
            temp = temp->hashNext;
        }
        fprintf(out, "\n");
    }
    fprintf(out, "--------------------\n");
}

template <class K, class V> char Dictionary<K, V>::iterate(int& index, int& offset, V& value) const
{
    DNode<K, V>* temp;

    if ((index < 0) || (index >= getSize())) {
        return false;
    }

    temp = table[index];
    while ((temp == NULL) && ((++index) < getSize())) {
        temp = table[index];
        offset = 0;
    }

    if (temp == NULL) {
        return false;
    }
    unsigned i = 0;
    while ((temp != NULL) && (i < offset)) {
        temp = temp->hashNext;
        i++;
    }

    if (temp == NULL) {
        return false;
    }

    value = temp->value;
    if (temp->hashNext == NULL) {
        index++;
        offset = 0;
    } else {
        offset++;
    }

    return true;
}

template <class K, class V> unsigned Dictionary<K, V>::getSize(void) const
{
    return size;
}

template <class K, class V> unsigned Dictionary<K, V>::getEntries(void) const
{
    return entries;
}

template <class K, class V> char Dictionary<K, V>::contains(K& key)
{
    int offset;
    DNode<K, V>* node;

    offset = keyHash(key);

    node = table[offset];

    if (node == NULL) {
        return false;
    }

    while (node != NULL) {
        if ((node->key) == key) {
            return true;
        }
        node = node->hashNext;
    }
    return false;
}

template <class K, class V> char Dictionary<K, V>::updateValue(K& key, V& value)
{
    int retval;

    retval = remove(key);
    if (retval == false) {
        return false;
    }

    add(key, value);
    return true;
}

template <class K, class V> char Dictionary<K, V>::add(K& key, V& value)
{
    int offset;
    DNode<K, V>*node, *item, *temp;
    float percent;

    item = (DNode<K, V>*)new DNode<K, V>;
    assert(item != NULL);

    memcpy(&(item->key), &key, sizeof(K));
    memcpy(&(item->value), &value, sizeof(V));

    item->hashNext = NULL;

    remove(key);

    offset = keyHash(key);

    node = table[offset];

    if (node == NULL) {
        table[offset] = item;
    } else {
        temp = table[offset];
        table[offset] = item;
        item->hashNext = temp;
    }

    entries++;
    percent = entries;
    percent /= (float)getSize();
    if (percent >= EXPAND_THRESHOLD) {
        expand();
    }

    return true;
}

template <class K, class V> char Dictionary<K, V>::remove(K& key, V& value)
{
    int offset;
    DNode<K, V>*node, *last, *temp;
    float percent;

    if (entries == 0) {
        return false;
    }

    percent = (entries - 1);
    percent /= (float)getSize();

    offset = keyHash(key);
    node = table[offset];

    last = node;
    if (node == NULL) {
        return false;
    }

    if (0 == memcmp(&(node->key), &key, sizeof(K))) {
        memcpy(&value, &(node->value), sizeof(V));
        temp = table[offset]->hashNext;
        delete (table[offset]);
        table[offset] = temp;
        entries--;
        if (percent <= SHRINK_THRESHOLD)
            shrink();
        return true;
    }
    node = node->hashNext;

    while (node != NULL) {
        if (0 == memcmp(&(node->key), &key, sizeof(K))) {
            memcpy(&value, &(node->value), sizeof(V));
            last->hashNext = node->hashNext;
            entries--;
            delete (node);
            break;
        }
        last = node;
        node = node->hashNext;
    }

    if (percent <= SHRINK_THRESHOLD)
        shrink();
    return true;
}

template <class K, class V> char Dictionary<K, V>::remove(K& key)
{
    V temp;
    return remove(key, temp);
}

template <class K, class V> char Dictionary<K, V>::removeAny(K& key, V& value)
{
    int offset;
    DNode<K, V>*node, *last, *temp;
    float percent;

    if (entries == 0) {
        return false;
    }

    percent = (entries - 1);
    percent /= (float)getSize();

    int i;
    offset = -1;
    for (i = 0; i < (int)getSize(); i++)
        if (table[i] != NULL) {
            offset = i;
            break;
        }

    if (offset == -1) {
        return false;
    }

    node = table[offset];
    last = node;

    memcpy(&key, &(node->key), sizeof(K));
    memcpy(&value, &(node->value), sizeof(V));

    temp = table[offset]->hashNext;
    delete (table[offset]);
    table[offset] = temp;
    entries--;
    if (percent <= SHRINK_THRESHOLD) {
        shrink();
    }
    return true;
}

template <class K, class V> char Dictionary<K, V>::getValue(K& key, V& value)
{
    int offset;
    DNode<K, V>* node;

    offset = keyHash(key);

    node = table[offset];

    if (node == NULL) {
        return false;
    }

    while ((node != NULL) && (memcmp(&(node->key), &key, sizeof(K)))) {
        node = node->hashNext;
    }

    if (node == NULL) {
        return false;
    }

    memcpy(&value, &(node->value), sizeof(V));
    return true;
}

template <class K, class V> void Dictionary<K, V>::shrink(void)
{
    int i;
    int oldsize;
    unsigned offset;
    DNode<K, V>**oldtable, *temp, *first, *next;

    if ((size <= (unsigned)MIN_TABLE_SIZE) || (keepSize == true)) {
        return;
    }
    oldtable = table;
    oldsize = size;
    size /= 2;
    tableBits--;

    table = (DNode<K, V>**)new DNode<K, V>*[size];
    assert(table != NULL);
    memset((void*)table, 0, size * sizeof(void*));

    for (i = 0; i < oldsize; i++) {
        temp = oldtable[i];
        while (temp != NULL) {
            offset = keyHash(temp->key);
            first = table[offset];
            table[offset] = temp;
            next = temp->hashNext;
            temp->hashNext = first;
            temp = next;
        }
    }

    delete[](oldtable);
}

template <class K, class V> void Dictionary<K, V>::expand(void)
{
    int i;
    int oldsize;
    unsigned offset;
    DNode<K, V>**oldtable, *temp, *first, *next;

    if (keepSize == true) {
        return;
    }

    oldtable = table;
    oldsize = size;
    size *= 2;
    tableBits++;

    table = (DNode<K, V>**)new DNode<K, V>*[size];
    assert(table != NULL);
    memset((void*)table, 0, size * sizeof(void*));

    for (i = 0; i < oldsize; i++) {
        temp = oldtable[i];
        while (temp != NULL) {
            offset = keyHash(temp->key);
            first = table[offset];
            table[offset] = temp;
            next = temp->hashNext;
            temp->hashNext = first;
            temp = next;
        }
    }

    delete[](oldtable);
}

#endif
