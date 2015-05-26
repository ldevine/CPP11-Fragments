#ifndef HASHTABLE_H
#define HASHTABLE_H

//--------------------------------------------------------------------------
// Author: Lance De Vine
//
// Port of Jeff Preshing's "World's Simplest Lock-Free
// Hash Table"
// http://preshing.com/20130605/the-worlds-simplest-lock-free-hash-table/
//
// Code is largely the same as the original but uses C++11 atomics
//--------------------------------------------------------------------------

#include <atomic>
#include <iostream>

#define USE_FAST_SETITEM 0

using std::endl;
using std::cout;

class HashTable
{
public:
	struct Entry
	{
		std::atomic<uint32_t> key;
		std::atomic<uint32_t> value;
	};


public:

	HashTable(uint32_t arraySize) {

		// Initialize cells
		assert((arraySize & (arraySize - 1)) == 0); // Must be a power of 2
		m_arraySize = arraySize;
		m_entries = new Entry[arraySize];
		
		Clear();

		cout << "HahsMap created ..." << endl;
	}

	~HashTable() {
		// Delete cells
		delete[] m_entries;
	}

	// Basic operations
#if USE_FAST_SETITEM
	void SetItem(uint32_t key, uint32_t value)
	{
		assert(key != 0);
		assert(value != 0);

		for (uint32_t idx = integerHash(key);; idx++)
		{
			idx &= m_arraySize - 1;

			// Load the key that was there.
			uint32_t probedKey = m_entries[idx].key.load();
			if (probedKey != key)
			{
				// The entry was either free, or contains another key.
				if (probedKey != 0)
					continue; // Usually, it contains another key. Keep probing.

				expected = 0;

				// The entry was free. Now let's try to take it using a CAS.
				if (m_entries[idx].key.compare_exchange_strong(expected, key)) {
					m_entries[idx].value.store(value);
					return;
				}
				/*
				if ((expected != 0) && (expected != key))
					continue; // Another thread just stole it from underneath us.
				// Either we just added the key, or another thread did. 
				*/
			}

			// Store the value in this array entry.
			m_entries[idx].value.store(value);
			return;
		}
	}
#else
	void SetItem(uint32_t key, uint32_t value)
	{
		assert(key != 0);
		assert(value != 0);

		for (uint32_t idx = integerHash(key);; idx++)  {
			idx &= m_arraySize - 1;
			//cout << idx << endl;
			expected = 0;
			m_entries[idx].key.compare_exchange_strong(expected, key);
			if ((expected == 0) || (expected == key)) {
				m_entries[idx].value.store(value);
				return;
			}

		}
	}
#endif

	//----------------------------------------------
	uint32_t GetItem(uint32_t key)
	{
		assert(key != 0);
		for (uint32_t idx = integerHash(key);; idx++)
		{
			idx &= m_arraySize - 1;
			uint32_t probedKey = m_entries[idx].key.load();
			if (probedKey == key)
				return m_entries[idx].value.load();
			if (probedKey == 0)
				return 0;
		}
	}

	uint32_t GetItemCount() {
		
		uint32_t itemCount = 0;

		for (uint32_t idx = 0; idx < m_arraySize; idx++)
		{

			if (m_entries[idx].key.load() != 0
				&& m_entries[idx].value.load() != 0)
				itemCount++;
		}
		return itemCount;
	}

	void Clear() {

		memset(m_entries, 0, sizeof(Entry) * m_arraySize);
	}

private:
	Entry* m_entries;
	uint32_t m_arraySize;
	unsigned int expected;

	//----------------------------------------------
	// from code.google.com/p/smhasher/wiki/MurmurHash3
	inline static uint32_t integerHash(uint32_t h)
	{
		h ^= h >> 16;
		h *= 0x85ebca6b;
		h ^= h >> 13;
		h *= 0xc2b2ae35;
		h ^= h >> 16;
		return h;
	}

};







#endif


