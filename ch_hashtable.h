#pragma once
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

namespace ch
{
    uint32_t StringHash(char *Key)
    {
        uint32_t Hash = 5381;
        
        int C = *Key++;
        while (C)
        {
            Hash = ((Hash << 5) + Hash) + C; /* hash * 33 + c */
            C = *Key++;
        }
        
        return Hash;
    }
    
    template <typename T>
        struct entry
    {
        char *Key;
        T Data;
    };
    
    template <typename T>
        struct hash_table
    {
        entry<T> *Entries;
        int Size;
        int Cap;
        
        void Push(char *Key, T Entry);
        bool Contains(char *Key);
        
        T operator[](char *Key) const;
        T &operator[](char *Key);
    };
    
    template <typename T>
        int ProbeForEmptySlot(entry<T> *Entries, char *Key, int Cap)
    {
        int InitialGuess = StringHash(Key) % Cap;
        
        if (!Entries[InitialGuess].Key)
        {
            return InitialGuess;
        }
        else
        {
            int Next = (InitialGuess + 1) % Cap;
            while (Next != InitialGuess)
            {
                if (!Entries[Next].Key)
                {
                    return Next;
                }
                Next += 1;
                if (Next == Cap) Next = 0;
            }
            
            return -1;
        }
    }
    
    template <typename T>
        int SearchKey(entry<T> *Entries, char *Key, int Cap)
    {
        int InitialGuess = StringHash(Key) % Cap;
        if (!Entries[InitialGuess].Key) return -1;
        
        if (strcmp(Entries[InitialGuess].Key, Key) == 0)
        {
            return InitialGuess;
        }
        else
        {
            int Next = (InitialGuess + 1) % Cap;
            while (Next != InitialGuess)
            {
                if (!Entries[Next].Key) return -1;
                
                if (strcmp(Entries[Next].Key, Key) == 0)
                {
                    return Next;
                }
                Next += 1;
                if (Next == Cap) Next = 0;
            }
            
            return -1;
        }
    }
    
    template <typename T>
        void hash_table<T>::Push(char *Key, T Entry)
    {
        if (2 * Size >= Cap)
        {
            size_t OldCap = Cap;
            
            if (Cap != 0)
            {
                //TODO(chen): want to ensure prime
                Cap = 2 * Cap + 1;
            }
            else
            {
                Cap = 1; // a large initial table size
            }
            
            entry<T> *NewEntries = (entry<T> *)calloc(Cap, sizeof(entry<T>));
            for (int I = 0; I < OldCap; ++I)
            {
                if (Entries[I].Key)
                {
                    int NewIndex = ProbeForEmptySlot(NewEntries, Entries[I].Key, Cap);
                    assert(NewIndex != -1);
                    NewEntries[NewIndex] = Entries[I];
                }
            }
            free(Entries);
            Entries = NewEntries;
        }
        
        int EntryIndex = ProbeForEmptySlot(Entries, Key, Cap);
        assert(EntryIndex != -1);
        Entries[EntryIndex].Key = strdup(Key);
        Entries[EntryIndex].Data = Entry;
        
        Size += 1;
    }
    
    template <typename T>
        bool hash_table<T>::Contains(char *Key)
    {
        if (Size == 0) return false;
        return SearchKey(Entries, Key, Cap) != -1;
    }
    
    template <typename T>
        T& hash_table<T>::operator[](char *Key)
    {
        int KeyIndex = SearchKey(Entries, Key, Cap);
        assert(KeyIndex != -1);
        return Entries[KeyIndex].Data;
    }
    
    template <typename T>
        T hash_table<T>::operator[](char *Key) const
    {
        return operator[](Key);
    }
};