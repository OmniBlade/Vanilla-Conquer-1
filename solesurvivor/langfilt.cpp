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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "langfilt.h"
#include <stdint.h>

#define FILTER_MAXCHAR 256
#define MAX_INT_LEN    13

// 200 characters of blat.
static const char Blat[] =
    "@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/"
    "^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^@%#!&*{!/^";

void AllLower(char* String, int size);

/***********************************************************************************************
 *  LanguageFilterClass::LanguageFilterClass --  Language filter. 									  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/05/96 JB Created                                                                       *
 *   08/20/96 JB Re-written                                                                    *
 *=============================================================================================*/
LanguageFilterClass::LanguageFilterClass(void)
{

    // Initialize
    memset(Hash_Table, 0, sizeof(Hash_Table));

    Substrings = NULL;

    Total_Number_Of_Substrings = 0;
}

/***********************************************************************************************
 *  LanguageFilterClass::~LanguageFilterClass --  Destructor                                   *
 *                                                                                             *
 * INPUT:   See below:                                                                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/02/96 JB Created                                                                       *
 *   08/20/96 JB Re-written                                                                    *
 *=============================================================================================*/

LanguageFilterClass::~LanguageFilterClass()
{
    SubStringType *sub, *deleteMe;

    // 11/05/96 DMA.  JB tried a delete[] on a linked-list.  What a maroon...
    // Loop through each SubStringType in the link list, and delete it (that's
    // the *correct* way to do it)
    sub = Substrings;
    while (sub) {
        deleteMe = sub;
        if (sub->Next) {
            sub = sub->Next;
        } else {
            sub = NULL;
        }
        delete deleteMe;
    }
}

/***********************************************************************************************
 *  LanguageFilterClass::Init -- Initialize the internal datastructures				  		   *
 *                                                                                             *
 * INPUT:   A null terminated string with lines separated by \n, and a length                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/20/96 JB Created                                                                       *
 *=============================================================================================*/
bool LanguageFilterClass::Init(const char* buffer, int length)
{
    char* local_string;

    local_string = new char[length + 1];
    strncpy(local_string, buffer, length);

    // Loop and parse in all of the word and word pairs
    char* ptr = strtok(local_string, "\n");
    while (ptr) {
        // If there is a space in this line, it is a whole word replacement
        // If not, it's a substring search

        // In any case, decode it, and make it lower
        decode(ptr);
        AllLower(ptr, strlen(ptr));

        if (strchr(ptr, ' ') != NULL) {
            hashit(ptr);
            Total_Number_Of_Words++;
        } else {
            addit(ptr);
            Total_Number_Of_Substrings++;
        }

        ptr = strtok(NULL, "\n");

    } // end of while

    delete[] (local_string);

    return true;
}

/***********************************************************************************************
 *  LanguageFilterClass::Init -- Initialize the internal datastructures				  		   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/20/96 JB Created                                                                       *
 *=============================================================================================*/
bool LanguageFilterClass::Init(const char* file_name)
{
    FILE* language_file_ptr;
    char buffer[256];

    // Open the file
    if ((language_file_ptr = fopen(file_name, "r")) != NULL) {

        // Loop and read in all of the word and word pairs
        fgets(buffer, MAX_WORD_LEN, language_file_ptr);

        while (!feof(language_file_ptr)) {
            // If there is a space in this line, it is a whole word replacement
            // If not, it's a substring search

            // In any case, decode it, and make it lower
            decode(buffer);
            AllLower(buffer, strlen(buffer));
            if (buffer[0] != NULL) {
                if (strchr(buffer, ' ') != NULL) {
                    hashit(buffer);
                    Total_Number_Of_Words++;
                } else {
                    addit(buffer);
                    Total_Number_Of_Substrings++;
                }
            }
            fgets(buffer, MAX_WORD_LEN, language_file_ptr);
        }

        fclose(language_file_ptr);
    }

    return true;
}

/***********************************************************************************************
 *  LanguageFilterClass::Add_Users_Words -- Add user-defined word list                         *
 *                                                                                             *
 * INPUT:   file name that contains words the user wants to filter                             *
 *                                                                                             *
 * OUTPUT:  true if words added successfully, false if failure                                 *
 *                                                                                             *
 * WARNINGS:  If an error occurs in one of the words (eg. too long), then the return value is  *
 *   false, even if other words have been successfully added/hashed before the erroneous word. *
 *   The words successfully added WILL be filtered.  Also, there is no way to remove the words *
 *   we added, so if you call this function twice, duplicates may occur.  (Ask Jeff B.  I'm    *
 *   not sure if the hashing function avoids duplicates.)                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/96 DMA Created                                                                      *
 *=============================================================================================*/
bool LanguageFilterClass::Add_Users_Words(const char* /* file_name */)
{
    return true;
}

/***********************************************************************************************
 *  LanguageFilterClass::Filter --  Actually filters a string                                  *
 *                                                                                             *
 * INPUT:   See below:                                                                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/02/96 JB Created                                                                       *
 *   08/20/96 JB Re-written                                                                    *
 *=============================================================================================*/

bool LanguageFilterClass::Filter(char* message, FilterStrengthType strength)
{

    bool return_value = false;

    // Early exit?
    if (((Total_Number_Of_Words == 0) && (Total_Number_Of_Substrings == 0)) || (message == NULL)) {
        return return_value;
    }

    // General algorithm:
    // Check the strength, if whole word, loop over the words, check each one
    // If substring, blat the offending word if found

    if (strength & FST_SUBSTRING) {
        char* found_ptr;

        while ((found_ptr = substring_search(message, strlen(message))) != NULL) {
            // Find the previous white space or BOS
            while (((found_ptr) > message) && (!isspace(*(found_ptr - 1)))) {
                found_ptr--;
            }

            // Count to the next white space or EOS
            int length = 1;
            while ((*(found_ptr + length) != '\0') && (!isspace(*(found_ptr + length)))
                   && (*(found_ptr + length) != '\n')) {
                length++;
            }

            // Copy the blat into the passed string
            // calc the pointer into the message
            intptr_t startindex = (((intptr_t)found_ptr) - ((intptr_t)message));
            char* start = message + startindex;
            strncpy(start, Blat, length);
            return_value = true;
        }
    }

    if (strength & FST_WORD) {
        char* found_ptr;
        char* end_ptr;
        char* replacement_ptr = NULL;

        char* word_ptr = message;
        char* word_buf = new char[strlen(message) + 1];

        // Loop over the words and look them up. If found, replace them
        while (*word_ptr && (*word_ptr != '\n')) {
            // Make a word

            end_ptr = word_ptr;
            while (*end_ptr && !isspace(*end_ptr) && (*end_ptr != '\r') && (*end_ptr != '\n')) {
                end_ptr++;
            }

            intptr_t length = ((intptr_t)end_ptr) - ((intptr_t)word_ptr);
            if (length > 0) {
                strncpy(word_buf, word_ptr, length);
                word_buf[length] = '\0';

                found_ptr = word_search(word_buf, &replacement_ptr);

                if (found_ptr && replacement_ptr) {
                    strncpy(word_ptr, replacement_ptr, strlen(replacement_ptr));
                    return_value = true;
                }

                word_ptr += length;
            } else {
                word_ptr++;
            }
            replacement_ptr = found_ptr = NULL;
        }

        delete[] word_buf;
    }

    return return_value;
}

/***********************************************************************************************
 *  LanguageFilterClass::hashit --  hash this word into the table                              *
 *                                                                                             *
 * INPUT:   Two words                                                                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/20/96 JB Created                                                                       *
 *=============================================================================================*/

void LanguageFilterClass::hashit(char* words)
{
    char* sourceword;
    char* destword;
    unsigned long hash;
    HashBucketType *hash_entry, *last_hash_entry, *this_hash_entry;

    if (words == NULL) {
        return;
    }

    // Strip off the words
    sourceword = destword = NULL;
    sourceword = words;
    char* ptr = strchr(words, ' ');
    if (ptr) {
        *ptr = NULL;
        destword = ptr + 1;
    }

    // These shouldn't happen, but check anyway
    if (strlen(sourceword) > MAX_WORD_LEN) {
        sourceword[MAX_WORD_LEN] = '\0';
    }
    if (strlen(destword) > MAX_WORD_LEN) {
        destword[MAX_WORD_LEN] = '\0';
    }

    // Do a hash search to get a pointer to the bucket
    hash_entry = hashsearch(sourceword, &hash);
    if (hash_entry == NULL) {
        // Didn't find this entry. Check the hash bucket
        hash_entry = &Hash_Table[hash % HASH_TABLE_SIZE];

        if (hash_entry && hash_entry->Source_Word[0] != NULL) {
            // Add it to the end of the bucket chain
            // This is a little inefficient in that we need to follow the chain again, but
            // it's pretty inexpensive
            do {
                last_hash_entry = hash_entry;
                hash_entry = hash_entry->Next;
            } while (hash_entry);

            // last_hash_entry cannot be == NULL! but check anyway
            if (last_hash_entry != NULL) {
                // Create this record
                this_hash_entry = new HashBucketType;
                strcpy(this_hash_entry->Source_Word, sourceword);
                strcpy(this_hash_entry->Dest_Word, destword);
                this_hash_entry->Next = NULL;

                // Link it in
                last_hash_entry->Next = this_hash_entry;
            }
        } else {
            // Didn't find it, set this bucket's hash entry.
            // We know a little too much about the hash table right now, but that's OK
            this_hash_entry = &Hash_Table[hash % HASH_TABLE_SIZE];
            strcpy(this_hash_entry->Source_Word, sourceword);
            strcpy(this_hash_entry->Dest_Word, destword);
            this_hash_entry->Next = NULL;
        }
    }
}

/***********************************************************************************************
 *  LanguageFilterClass::addit  --  add this word to the substring list                        *
 *                                                                                             *
 * INPUT:   A word                                                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/20/96 JB Created                                                                       *
 *=============================================================================================*/

void LanguageFilterClass::addit(char* word)
{

    if (word == NULL) {
        return;
    }

    // Create a new link, copy this into it.
    SubStringType* newsubstring = new SubStringType;
    strncpy(newsubstring->Source_String, word, MAX_WORD_LEN);
    newsubstring->Source_String[MAX_WORD_LEN] = '\0';

    // Prepend it to the list.
    newsubstring->Next = Substrings;
    Substrings = newsubstring;
}

/***********************************************************************************************
 *  LanguageFilterClass::decode --  Decode the word                                            *
 *                                                                                             *
 * INPUT:   See below:                                                                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/02/96 JB Created                                                                       *
 *   08/20/96 JB Re-written                                                                    *
 *=============================================================================================*/
void LanguageFilterClass::decode(char* word)
{
    while (*word) {
        *word = *word - 10;
        word++;
    }
}

/***********************************************************************************************
 *  LanguageFilterClass::bmsearch -- Perform a boyer-moore search                              *
 *                                                                                             *
 * INPUT:   A substring to search for, a source string, and it's length                        *
 *                                                                                             *
 * OUTPUT:  A pointer to the found substring or NULL                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/20/96 JB Created                                                                       *
 *=============================================================================================*/
char* LanguageFilterClass::bmsearch(char* substring, char* source, int length)
{
    int subindex, index2, index, substring_len, skiparray[FILTER_MAXCHAR];

    // Boyer-Moore Search
    substring_len = strlen(substring);
    if (substring_len == 0) {
        return (source);
    }

    // Initialize the skip array with the default skip (length of the substring)
    for (index = 0; index < FILTER_MAXCHAR; index++) {
        skiparray[index] = substring_len;
    }

    // Adjust the values of the substring characters themselves
    for (index = 0; index < substring_len - 1; index++) {
        skiparray[substring[index]] = substring_len - index - 1;
    }

    // Search the string using Boyer-Moore
    for (index = substring_len - 1; index < length; index += skiparray[source[index] & (FILTER_MAXCHAR - 1)]) {
        for (index2 = substring_len - 1, subindex = index; index2 >= 0 && source[subindex] == substring[index2];
             index2--) {
            subindex--;
        }

        // If found (dec'd beyond the front of the substring, return this index
        if (index2 == (-1)) {
            return (source + subindex + 1);
        }
    }

    // Not found.
    return (NULL);
}

/***********************************************************************************************
 *  LanguageFilterClass::hashsearch -- Returns a bucket for the passed word                    *
 *                                                                                             *
 * INPUT:   A word                                                                             *
 *                                                                                             *
 * OUTPUT:  A hashbucket                                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/20/96 JB Created                                                                       *
 *=============================================================================================*/
HashBucketType* LanguageFilterClass::hashsearch(char* word, unsigned long* hash_value)
{
    // Get the source word's hash value
    int cnt, length, padded_length;
    unsigned long hash;
    HashBucketType* hash_entry;
    char* local_string;

    // Create a local padded to the nearest dword and memset it, copy and make it lower case
    length = strlen(word);
    padded_length = length + ((4 - (length & 3)) & 3);
    local_string = new char[padded_length + 1];
    memset(local_string, 0, padded_length + 1);
    strcpy(local_string, word);
    AllLower(local_string, length);

    // Calculate the hash value
    for (cnt = 0, hash = 0; cnt < length; cnt += 4) {
        hash += *((long*)(local_string + cnt));
    }
    // Return the hash value;
    *hash_value = hash;

    // Grab this hash table entry, search it for the word using boyer-moore
    hash_entry = &(Hash_Table[hash % HASH_TABLE_SIZE]);
    while (hash_entry && (hash_entry->Source_Word[0] != NULL)) {
        if (bmsearch(hash_entry->Source_Word, local_string, strlen(local_string)) != NULL) {
            delete[] local_string;
            return (hash_entry);
        }
        hash_entry = hash_entry->Next;
    }

    // Return a failure
    delete[] local_string;
    return (NULL);
}

/***********************************************************************************************
 *  LanguageFilterClass::word_search -- Search for a whole word                                *
 *                                                                                             *
 * INPUT:   A word and it's length                                                             *
 *                                                                                             *
 * OUTPUT:  A pointer to the replacement string or NULL if not found                           *
 *                                                                                             *
 * WARNINGS:   Don't free the replacement string, ever!                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/20/96 JB Created                                                                       *
 *=============================================================================================*/
char* LanguageFilterClass::word_search(char* word, char** replacement)
{
    unsigned long hash_value; // Dummy
    HashBucketType* hash_entry;

    // Given a word, hash it into the table and search the buckets
    hash_entry = hashsearch(word, &hash_value);
    if (hash_entry != NULL) {
        *replacement = hash_entry->Dest_Word;
        return (hash_entry->Source_Word);
    }
    return (NULL);
}

/***********************************************************************************************
 *  LanguageFilterClass::substring_search -- search for a substring                            *
 *                                                                                             *
 * INPUT:   A string and it's length                                                           *
 *                                                                                             *
 * OUTPUT:  A pointer to the found string or NULL if not found                                 *
 *                                                                                             *
 * WARNINGS:   Never free the returned value!                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/20/96 JB Created                                                                       *
 *=============================================================================================*/
char* LanguageFilterClass::substring_search(char* source, int length)
{
    SubStringType* substring_ptr;
    char* ptr;
    char* local_string;

    // Make a copy of the string, make it lower case so we're comparing apples with apples
    // Then normalize the return pointer with respect to the passed source string
    local_string = strdup(source);
    AllLower(local_string, strlen(local_string));

    substring_ptr = Substrings;
    while (substring_ptr) {
        if ((ptr = bmsearch(substring_ptr->Source_String, local_string, length)) != NULL) {
            free(local_string);

            // Normalizing - Get the index into local string and add that index to source
            intptr_t index = (((intptr_t)ptr) - ((intptr_t)local_string));
            return (source + index);
        }

        substring_ptr = substring_ptr->Next;
    }
    free(local_string);
    return (NULL);
}

void AllLower(char* String, int size)
{
    int Cnt;

    for (Cnt = 0; Cnt < size; Cnt++) {
        String[Cnt] = (char)tolower(String[Cnt]);
    }
}
