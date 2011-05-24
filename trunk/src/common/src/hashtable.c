/* Copyright (C) 2004 Christopher Clark <firstname.lastname@cl.cam.ac.uk> */
/* @par[Revision] $Id$
 **/

#include "hashtable.h"
#include "hashtable_private.h"
#include "procTypes.h"

#define CEIL(VARIABLE) ( (VARIABLE - (unsigned int)VARIABLE)==0 ? (unsigned int)VARIABLE : (unsigned int) VARIABLE+1 )

/*
Credit for primes table: Aaron Krowne
 http://br.endernet.org/~akrowne/
 http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
*/
static const uint32_t primes[] = {
53, 97, 193, 389,
769, 1543, 3079, 6151,
12289, 24593, 49157, 98317,
196613, 393241, 786433, 1572869,
3145739, 6291469, 12582917, 25165843,
50331653, 100663319, 201326611, 402653189,
805306457, 1610612741
};
const unsigned int prime_table_length = sizeof(primes)/sizeof(primes[0]);
const float max_load_factor = 0.65F;

uint32_t djbHash(StringType* str)
{
	char* tmp = (char*) str->str;
	uint32_t hash = 5381;
	unsigned int i = 0;

	for(i = 0; i < sizeof(CharType)*str->length; tmp++, i++)
	{
		hash = ((hash << 5) + hash) + (*tmp);
	}

	return hash;
}

/*****************************************************************************/

struct hashtable * create_hashtable(unsigned int minsize,
						uint32_t (*hashfn) (StringType* key),
						char (*eqfn) (const StringType str1, const StringType str2))
{
    struct hashtable *h;
    unsigned int pindex, size = primes[0];
    /* Check requested hashtable isn't too large */
    if (minsize > MAX_HASH_TABLE_SIZE) return NULL;
    /* Enforce size as prime */
    for (pindex=0; pindex < prime_table_length; pindex++) {
        if (primes[pindex] > minsize) { size = primes[pindex]; break; }
    }
    h = (struct hashtable *)EXIP_MALLOC(sizeof(struct hashtable));
    if (NULL == h) return NULL; /*oom*/
    h->table = (struct entry **)EXIP_MALLOC(sizeof(struct entry*) * size);
    if (NULL == h->table) { EXIP_MFREE(h); return NULL; } /*oom*/
    memset(h->table, 0, size * sizeof(struct entry *));
    h->tablelength  = size;
    h->primeindex   = pindex;
    h->entrycount   = 0;
    h->hashfn       = hashfn;
    h->eqfn         = eqfn;
    h->loadlimit    = CEIL(size * max_load_factor);
    return h;
}

/*****************************************************************************/

// Currently this function is not used. Some performance tests must be written
// and performed to show if it is better when it is used
//uint32_t hash(struct hashtable *h, void *k, unsigned int len)
//{
//    /* Aim to protect against poor hash functions by adding logic here
//     * - logic taken from java 1.4 hashtable source */
//    uint32_t i = h->hashfn(k, len);
//    i += ~(i << 9);
//    i ^=  ((i >> 14) | (i << 18)); /* >>> */
//    i +=  (i << 4);
//    i ^=  ((i >> 10) | (i << 22)); /* >>> */
//    return i;
//}

/*****************************************************************************/
static int hashtable_expand(struct hashtable *h)
{
    /* Double the size of the table to accommodate more entries */
    struct entry **newtable;
    struct entry *e;
    struct entry **pE;
    unsigned int newsize, i, index;
    /* Check we're not hitting max capacity */
    if (h->primeindex == (prime_table_length - 1) || primes[h->primeindex + 1] > MAX_HASH_TABLE_SIZE) return 0;
    newsize = primes[++(h->primeindex)];

    newtable = (struct entry **)EXIP_MALLOC(sizeof(struct entry*) * newsize);
    if (NULL != newtable)
    {
        memset(newtable, 0, newsize * sizeof(struct entry *));
        /* This algorithm is not 'stable'. ie. it reverses the list
         * when it transfers entries between the tables */
        for (i = 0; i < h->tablelength; i++) {
            while (NULL != (e = h->table[i])) {
                h->table[i] = e->next;
                index = indexFor(newsize,e->hash);
                e->next = newtable[index];
                newtable[index] = e;
            }
        }
        EXIP_MFREE(h->table);
        h->table = newtable;
    }
    /* Plan B: realloc instead */
    else 
    {
        newtable = (struct entry **) EXIP_REALLOC(h->table, newsize * sizeof(struct entry *));
        if (NULL == newtable) { (h->primeindex)--; return 0; }
        h->table = newtable;
        memset(newtable[h->tablelength], 0, newsize - h->tablelength);
        for (i = 0; i < h->tablelength; i++) {
            for (pE = &(newtable[i]), e = *pE; e != NULL; e = *pE) {
                index = indexFor(newsize,e->hash);
                if (index == i)
                {
                    pE = &(e->next);
                }
                else
                {
                    *pE = e->next;
                    e->next = newtable[index];
                    newtable[index] = e;
                }
            }
        }
    }
    h->tablelength = newsize;
    h->loadlimit   = CEIL(newsize * max_load_factor);
    return -1;
}

/*****************************************************************************/
unsigned int hashtable_count(struct hashtable *h)
{
    return h->entrycount;
}

/*****************************************************************************/
errorCode hashtable_insert(struct hashtable *h, StringType* key, size_t value)
{
    /* This method allows duplicate keys - but they shouldn't be used */
    unsigned int index;
    struct entry *e;
    if (++(h->entrycount) > h->loadlimit)
    {
        /* Ignore the return value. If expand fails, we should
         * still try cramming just this value into the existing table
         * -- we may not have memory for a larger table, but one more
         * element may be ok. Next time we insert, we'll try expanding again.*/
        hashtable_expand(h);
    }
    e = (struct entry *)EXIP_MALLOC(sizeof(struct entry));
    if (NULL == e) { --(h->entrycount); return MEMORY_ALLOCATION_ERROR; } /*oom*/
    e->hash = h->hashfn(key); // hash(h,k, len);
    index = indexFor(h->tablelength,e->hash);
    e->key = key;
    e->value = value;
    e->next = h->table[index];
    h->table[index] = e;
    return ERR_OK;
}

/*****************************************************************************/
size_t hashtable_search(struct hashtable *h, StringType* key)
{
    struct entry *e;
    uint32_t hashvalue;
    unsigned int index;
    hashvalue = h->hashfn(key); // hash(h,k, len);
    index = indexFor(h->tablelength,hashvalue);
    e = h->table[index];
    while (NULL != e)
    {
        /* Check hash value to short circuit heavier comparison */
        if ((hashvalue == e->hash) && h->eqfn(*key, *(e->key))) return e->value;
        e = e->next;
    }
    return SIZE_MAX;
}

/*****************************************************************************/
size_t hashtable_remove(struct hashtable *h, StringType* key)
{
    /* TODO: consider compacting the table when the load factor drops enough,
     *       or provide a 'compact' method. */

    struct entry *e;
    struct entry **pE;
    size_t value;
    uint32_t hashvalue;
    unsigned int index;

    hashvalue = h->hashfn(key); // hash(h,k, len);
    index = indexFor(h->tablelength, hashvalue);
    pE = &(h->table[index]);
    e = *pE;
    while (NULL != e)
    {
        /* Check hash value to short circuit heavier comparison */
        if (hashvalue == e->hash && h->eqfn(*key, *(e->key)))
        {
            *pE = e->next;
            h->entrycount--;
            value = e->value;
 //           freekey(e->key);
            EXIP_MFREE(e);
            return value;
        }
        pE = &(e->next);
        e = e->next;
    }
    return SIZE_MAX;
}

/*****************************************************************************/
/* destroy */
void hashtable_destroy(struct hashtable *h)
{
    unsigned int i;
    struct entry *e, *f;
    struct entry **table = h->table;
	for (i = 0; i < h->tablelength; i++)
	{
		e = table[i];
		while (NULL != e)
		{ f = e; e = e->next; EXIP_MFREE(f); }
	}
    EXIP_MFREE(h->table);
    EXIP_MFREE(h);
}

/*
 * Copyright (c) 2002, Christopher Clark
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
