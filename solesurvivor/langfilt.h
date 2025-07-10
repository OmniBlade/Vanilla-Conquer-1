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
#ifndef LANGFILT_H
#define LANGFILT_H

#define MAX_WORD_LEN    70
#define HASH_TABLE_SIZE 101
#define LANG_FILE       "/nl.cfg"

// For the whole word search
struct HashBucketType
{
    char Source_Word[MAX_WORD_LEN + 1];
    char Dest_Word[MAX_WORD_LEN + 1];
    HashBucketType* Next;
};

// For the substring search
struct SubStringType
{
    char Source_String[MAX_WORD_LEN + 1];
    SubStringType* Next;
};

typedef char FilterStrengthType;
#define FST_WORD      0x01
#define FST_SUBSTRING 0x02
#define FST_ALL       FST_WORD | FST_SUBSTRING

class LanguageFilterClass
{
public:
    LanguageFilterClass(void);
    ~LanguageFilterClass(void);

    bool Init(char* file_name);          // Loads from a file
    bool Init(char* buffer, int length); // Loads from a buffer

    // 11/02/06 DMA
    bool Add_Users_Words(char* file_name); // Loads user-defined word list

    bool Filter(char* message, FilterStrengthType strength = FST_ALL);

private:
    // Whole word definitions
    HashBucketType Hash_Table[HASH_TABLE_SIZE];
    unsigned int Total_Number_Of_Words;

    // Substring definitions
    SubStringType* Substrings;
    unsigned int Total_Number_Of_Substrings;

    // Private functions
    void decode(char* word);
    void hashit(char* words);
    void addit(char* word);

    // These functions search the passed string for the target words
    char* word_search(char* source, char** replacement);
    char* substring_search(char* source, int sourcelength);

    // These functions are low level. They take a subtring and a source and find it
    char* bmsearch(char* substring, char* source, int sourcelength);
    HashBucketType* hashsearch(char* word, unsigned long* sourcelength);
};

#endif
