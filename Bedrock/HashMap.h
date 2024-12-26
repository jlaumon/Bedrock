// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Hash.h>
#include <Bedrock/Vector.h>


enum class EInsertResult : int8
{
	Found,		// The key was found.
	Added,		// The key was added to the map.
	Replaced	// The key was found in the map, and the value was replaced.
};


namespace Details
{
	struct HashMapBucket
	{
		static constexpr int cFingerprintBits = 8;
		static constexpr int cFingerprintMask = (1 << cFingerprintBits) - 1;
		static constexpr int cDistanceIncrement = 1 << cFingerprintBits;
		uint32	mDistanceAndFingerprint;	// Upper bits contain the distance to the ideal bucket, lower bits are from the hash.
		int		mKeyValueIndex;				// Index where to find the corresponding key-value.

		static uint32 sGetDistanceAndFingerprint(uint64 inHash)
		{
			uint32 fingerprint = inHash & cFingerprintMask;
			uint32 distance    = cDistanceIncrement; // Note: start at 1 because 0 means the bucket is unused.
			return fingerprint + distance;
		}
	};
}


template <typename taKey, typename taValue>
struct KeyValue
{
	taKey	mKey;
	taValue mValue;
};


template <typename taKey, typename taValue>
struct MapInsertResult
{
	MapInsertResult(KeyValue<taKey, taValue>& ioKeyValue, EInsertResult inResult)
		: mKey(ioKeyValue.mKey), mValue(ioKeyValue.mValue), mResult(inResult)
	{}

	const taKey&  mKey;
	taValue&      mValue;
	EInsertResult mResult;
};


template <typename taKey>
struct SetInsertResult
{
	SetInsertResult(const taKey& inKey, EInsertResult inResult)
		: mKey(inKey), mResult(inResult)
	{}

	const taKey&  mKey;
	EInsertResult mResult;
};


// Dense HashMap class.
// Heavily insipired from https://github.com/martinus/unordered_dense.
// The key-values are stored contiguously (no holes), so iteration is very fast. Bucket metadata is stored separately.
// Supports TempAllocator. Behaves as a set if taValue is void (see HashSet typedef) below.
template <
	typename taKey,
	typename taValue,
	typename taHash = Hash<taKey>,
	template <typename> typename taAllocator = DefaultAllocator
>
struct HashMap : taHash
{
	static constexpr bool cIsMap = !cIsVoid<taValue>;
	static constexpr bool cIsSet =  cIsVoid<taValue>;

	using KeyValue = Conditional<cIsMap, KeyValue<taKey, taValue>, taKey>;
	using InsertResult = Conditional<cIsMap, MapInsertResult<taKey, taValue>, SetInsertResult<taKey>>;

	using ConstIter = const KeyValue*;
	using Iter = const KeyValue*; // FIXME Iter should not allow modifying keys

	// Default
	HashMap() = default;
	~HashMap() = default;

	// Move
	HashMap(HashMap&&) = default;
	HashMap& operator=(HashMap&& ioOther) = default;

	// Copy
	HashMap(const HashMap& inOther);
	HashMap& operator=(const HashMap& inOther);

	void Clear();
	bool Empty() const { return mKeyValues.Empty(); }
	bool IsFull() const	{ return mKeyValues.Size() == mKeyValues.Capacity(); }

	int Size() const { return mKeyValues.Size(); }
	int Capacity() const { return mKeyValues.Capacity(); }

	ConstIter Begin() const { return mKeyValues.Begin(); }
	ConstIter End() const { return mKeyValues.End(); }
	Iter Begin() { return mKeyValues.Begin(); }
	Iter End() { return mKeyValues.End(); }
	ConstIter begin() const { return mKeyValues.Begin(); }
	ConstIter end() const { return mKeyValues.End(); }
	Iter begin() { return mKeyValues.Begin(); }
	Iter end() { return mKeyValues.End(); }

	// Find (non-const) ---------------------------------------

	Iter Find(const taKey& inKey) requires cIsMap
	{
		return FindInternal(inKey);
	}

	template <typename taAltKey>
	requires cIsTransparent<taHash>
	Iter Find(const taAltKey& inKey) requires cIsMap
	{
		return FindInternal(inKey);
	}

	// Find (const) -------------------------------------------

	ConstIter Find(const taKey& inKey) const
	{
		return FindInternal(inKey);
	}

	template <typename taAltKey>
	requires cIsTransparent<taHash>
	ConstIter Find(const taAltKey& inKey) const
	{
		return FindInternal(inKey);
	}


	// Contains -----------------------------------------------

	bool Contains(const taKey& inKey) const
	{
		return FindInternal(inKey) != End();
	}

	template <typename taAltKey>
	requires cIsTransparent<taHash>
	bool Contains(const taAltKey& inKey) const
	{
		return FindInternal(inKey) != End();
	}

	// Insert (Map version) -----------------------------------

	template <typename taAltValue>
	requires cIsAssignable<taValue&, taAltValue&&>
	InsertResult Insert(const taKey& inKey, taAltValue&& ioValue) requires cIsMap
	{
		return EmplaceInternal<EReplaceExisting::No>(inKey, gForward<taAltValue>(ioValue));
	}

	template <typename taAltValue>
	requires cIsAssignable<taValue&, taAltValue&&>
	InsertResult Insert(taKey&& ioKey, taAltValue&& ioValue) requires cIsMap
	{
		return EmplaceInternal<EReplaceExisting::No>(gMove(ioKey), gForward<taAltValue>(ioValue));
	}

	template <typename taAltKey, typename taAltValue>
	requires cIsTransparent<taHash> && cIsAssignable<taValue&, taAltValue&&>
	InsertResult Insert(taAltKey&& ioKey, taAltValue&& ioValue) requires cIsMap
	{
		return EmplaceInternal<EReplaceExisting::No>(gForward<taAltKey>(ioKey), gForward<taAltValue>(ioValue));
	}

	// Insert (Set version) -----------------------------------

	InsertResult Insert(const taKey& inKey) requires cIsSet
	{
		return EmplaceInternal<EReplaceExisting::No>(inKey);
	}

	InsertResult Insert(taKey&& ioKey) requires cIsSet
	{
		return EmplaceInternal<EReplaceExisting::No>(gMove(ioKey));
	}

	template <typename taAltKey>
	requires cIsTransparent<taHash>
	InsertResult Insert(taAltKey&& ioKey) requires cIsSet
	{
		return EmplaceInternal<EReplaceExisting::No>(gForward<taAltKey>(ioKey));
	}

	// InsertOrAssign (Map only) ------------------------------

	template <typename taAltValue>
	requires cIsAssignable<taValue&, taAltValue&&>
	InsertResult InsertOrAssign(const taKey& inKey, taAltValue&& ioValue) requires cIsMap
	{
		return EmplaceInternal<EReplaceExisting::Yes>(inKey, gForward<taAltValue>(ioValue));
	}

	template <typename taAltValue>
	requires cIsAssignable<taValue&, taAltValue&&>
	InsertResult InsertOrAssign(taKey&& ioKey, taAltValue&& ioValue) requires cIsMap
	{
		return EmplaceInternal<EReplaceExisting::Yes>(gMove(ioKey), gForward<taAltValue>(ioValue));
	}

	template <typename taAltKey, typename taAltValue>
	requires cIsTransparent<taHash> && cIsAssignable<taValue&, taAltValue&&>
	InsertResult InsertOrAssign(taAltKey&& ioKey, taAltValue&& ioValue) requires cIsMap
	{
		return EmplaceInternal<EReplaceExisting::Yes>(gForward<taAltKey>(ioKey), gForward<taAltValue>(ioValue));
	}

	// Emplace (Map and Set) ---------------------------------

	template <typename... taArgs>
	InsertResult Emplace(const taKey& inKey, taArgs&&... ioArgs)
	{
		return EmplaceInternal<EReplaceExisting::No>(inKey, gForward<taArgs>(ioArgs)...);
	}

	template <typename... taArgs>
	InsertResult Emplace(taKey&& ioKey, taArgs&&... ioArgs)
	{
		return EmplaceInternal<EReplaceExisting::No>(gMove(ioKey), gForward<taArgs>(ioArgs)...);
	}

	template <typename taAltKey, typename... taArgs>
	requires cIsTransparent<taHash>
	InsertResult Emplace(taAltKey&& ioKey, taArgs&&... ioArgs)
	{
		return EmplaceInternal<EReplaceExisting::No>(gForward<taAltKey>(ioKey), gForward<taArgs>(ioArgs)...);
	}

	// Operator[] (Map only) ---------------------------------

	template<class T = taValue>
	T& operator[](const taKey& inKey) requires cIsMap
	{
		return EmplaceInternal<EReplaceExisting::No>(inKey).mValue;
	}

	template<class T = taValue>
	T& operator[](taKey&& ioKey)
	{
		return EmplaceInternal<EReplaceExisting::No>(gMove(ioKey)).mValue;
	}

	template <typename taAltKey, class T = taValue>
	requires cIsTransparent<taHash>
	T& operator[](taAltKey&& ioKey) requires cIsMap 
	{
		return EmplaceInternal<EReplaceExisting::No>(gForward<taAltKey>(ioKey)).mValue;
	}

	// Erase (Map and Set) -----------------------------------

	bool Erase(const taKey& inKey)
	{
		return EraseInternal(inKey);
	}

	template <typename taAltKey>
	requires cIsTransparent<taHash>
	bool Erase(const taAltKey& inKey)
	{
		return EraseInternal(inKey);
	}

	Iter Erase(Iter inIter)
	{
		EraseInternal(GetKey(*inIter));
		return inIter;
	}

	// Reserve (Map and Set) -----------------------------------
	
	void Reserve(int inCapacity)
	{
		if (inCapacity <= Capacity())
			return;

		// Capacity is in number of KeyValues.
		// Number of buckets has to be a power of 2.
		int new_buckets_size = (int)gGetNextPow2(inCapacity);

		// Also we can only use ~80% of the buckets, so double the number again if that wouldn't fit.
		int num_key_values = new_buckets_size * 13 / 16; // 13/16 = 0.8125
		if (num_key_values < inCapacity)
			new_buckets_size *= 2;

		Grow(new_buckets_size);
	}

protected:
	using Bucket = Details::HashMapBucket;

	// Get the mask to use when incrementing bucket indices to get wrap-around.
	// The number of buckets is a power of 2, so we can use a bitwise and as a faster modulo.
	int GetBucketSizeMask() const
	{
		gAssert(!mBuckets.Empty());
		return mBuckets.Size() - 1;
	}

	// Helper to get the key (because of the KeyValue difference between Map/Set).
	const taKey& GetKey(const KeyValue& ioKeyValue) const
	{
		if constexpr (cIsMap)
			return ioKeyValue.mKey;
		else
			return ioKeyValue;
	}

	// Increase the capacity of the map.
	void Grow(int inNumBuckets)
	{
		gAssert(inNumBuckets == 0 || gIsPow2(inNumBuckets));
		gAssert(inNumBuckets == 0 || inNumBuckets > mBuckets.Size());

		int new_buckets_size = gMax(inNumBuckets, 16);
		int new_key_values_size = new_buckets_size * 13 / 16; // 13/16 = 0.8125

		// Free the buckets first to make sure the TempAllocator can grow the key-values allocation.
		mBuckets.ClearAndFreeMemory();
		mKeyValues.Reserve(new_key_values_size);

		// Re-allocate the buckets.
		mBuckets.Resize(new_buckets_size);

		// Fill the buckets.
		for (const KeyValue& key_value : mKeyValues)
		{
			// Find the right bucket index for this key.
			// Note: We know the key is not already present so we can skip some compares.
			bool key_may_be_found = false;
			auto [bucket_index, distance_and_fingerprint, _] = FindBucket(GetKey(key_value), key_may_be_found);

			// Insert the bucket.
			InsertBucket({ distance_and_fingerprint, mKeyValues.GetIndex(key_value) }, bucket_index);
		}
	}

	// Internal function to find a key.
	template <typename taAltKey>
	Iter FindInternal(const taAltKey& inKey) const
	{
		if (Empty()) [[unlikely]]
			return End();

		// Try to find the key.
		auto [bucket_index, _, found] = FindBucket(inKey);

		// If it was found, return an iterator.
		if (found)
			return mKeyValues.Begin() + mBuckets[bucket_index].mKeyValueIndex;

		// Otherwise return End.
		return End();
	}

	enum class EReplaceExisting
	{
		No,
		Yes,
	};

	// Internal function to emplace a key and value.
	template <EReplaceExisting taReplaceExisting, typename taAltKey, typename... taArgs>
	InsertResult EmplaceInternal(taAltKey&& ioKey, taArgs&&... ioArgs)
	{
		if (IsFull()) [[unlikely]]
			Grow(mBuckets.Size() * 2);

		// Try to find the key.
		auto [bucket_index, distance_and_fingerprint, found] = FindBucket(ioKey);

		if (found)
		{
			// Key already exist.
			KeyValue& key_value = mKeyValues[mBuckets[bucket_index].mKeyValueIndex];

			if constexpr (taReplaceExisting == EReplaceExisting::No || !cIsMap)
			{
				// Return the existing value.
				return { key_value, EInsertResult::Found };
			}
			else
			{
				// Replace the existing value.
				key_value.mValue = { gForward<taArgs>(ioArgs)... };
				return { key_value, EInsertResult::Replaced };
			}
		}

		// Key does not exist, add it.
		mKeyValues.EmplaceBack(gForward<taAltKey>(ioKey), gForward<taArgs>(ioArgs)...);

		// Insert a new bucket for it.
		Bucket new_bucket = { distance_and_fingerprint, mKeyValues.Size() - 1 };
		InsertBucket(new_bucket, bucket_index);

		KeyValue& key_value = mKeyValues.Back();
		return { key_value, EInsertResult::Added };
	}

	// Internal function to erase a key.
	template <typename taAltKey>
	bool EraseInternal(const taAltKey& inKey)
	{
		if (Empty()) [[unlikely]]
			return false;

		// Try to find the key.
		auto [bucket_index, distance_and_fingerprint, found] = FindBucket(inKey);

		if (found == false)
			return false; // Key does not exist.

		int key_value_index_to_erase = mBuckets[bucket_index].mKeyValueIndex;

		// Remove the corresponding bucket.
		EraseBucket(bucket_index);

		// If the key to erase is the last one, pop it and we're done.
		if (key_value_index_to_erase == mKeyValues.Size() - 1)
		{
			mKeyValues.PopBack();
			return true;
		}

		// Otherwise swap it with the last one, to minimize the number of moves.
		int last_key_value_index = mKeyValues.Size() - 1;

		// We also need to find the bucket of the key we will swap to update its index.
		const uint64 hash         = taHash::operator()(GetKey(mKeyValues.Back()));
		const int    buckets_mask = GetBucketSizeMask();
		bucket_index              = (int)hash & buckets_mask;

		while (true)
		{
			Bucket& bucket = mBuckets[bucket_index];

			// No need to compare fingerprints and keys, it's faster to just compare the key-value index. We know it will be found.
			if (bucket.mKeyValueIndex == last_key_value_index)
			{
				gAssert(bucket.mDistanceAndFingerprint != 0); // We should never encounter an empty bucket.

				// Found it, update the index.
				bucket.mKeyValueIndex = key_value_index_to_erase;
				break;
			}

			// Go to the next bucket.
			bucket_index = (bucket_index + 1) & buckets_mask;
		}

		// Swap-erase the key-value.
		mKeyValues.SwapErase(key_value_index_to_erase);

		return true;
	}

	
	struct FindBucketResult
	{
		int    mBucketIndex;			// The bucket where the key is or should be inserted.
		uint32 mDistanceAndFingerprint; // The distance and fingerprint of the key for this bucket.
		bool   mFoundKey;				// True if the key was found at this bucket.
	};

	// Find the bucket where a key is (or should be).
	template <typename taAltKey>
	FindBucketResult FindBucket(const taAltKey& inKey, bool inKeyMayBeFound = true) const
	{
		// Calculate the hash.
		const uint64 hash = taHash::operator()(inKey);

		// Get the ideal bucket index.
		const int buckets_mask = GetBucketSizeMask();
		int       bucket_index = (int)hash & buckets_mask;

		// Build the distance and fingerprint value. This is for Robin Hood hashing.
		// When inserting keys, we try to minimize the average distance to their ideal bucket.
		// If their ideal bucket is already used by another key, we either:
		// - go check the next bucket (meaning the distance to the idea bucket will be +1)
		// - steal the bucket and move the existing key to the next bucket instead (only if that existing
		// key distance was lower than the one we're trying to insert).
		// The fingerprint is a few bits from the hash that are also stored in the same value. The reasons
		// for doing that are:
		// - it saves having to compare the key (or the hash) since it's done at the same time as comparing the distance
		// - it forces an arbitrary order between keys that have the same distance (no need to iterate them all to be sure a key is not present)
		uint32 distance_and_fingerprint = Bucket::sGetDistanceAndFingerprint(hash);

		while (true)
		{
			Bucket bucket = mBuckets[bucket_index];

			// First check if the distance & fingerprint are equal.
			// Note: inKeyMayBeFound = false is a special case when growing the map where we know the key won't be found.
			if (inKeyMayBeFound && bucket.mDistanceAndFingerprint == distance_and_fingerprint) [[likely]]
			{
				// Then check if the key is equal too.
				if (GetKey(mKeyValues[bucket.mKeyValueIndex]) == inKey) [[likely]]
				{
					// Found it.
					return { bucket_index, distance_and_fingerprint, true };
				}
			}
			else if (bucket.mDistanceAndFingerprint < distance_and_fingerprint)
			{
				// Distance is lower, our key won't be found.
				// Note: This could be either because the bucket is used by another entry (closer to its ideal index) or because the bucket is empty.
				return { bucket_index, distance_and_fingerprint, false };
			}

			// Increment the distance and go to the next bucket.
			distance_and_fingerprint += Bucket::cDistanceIncrement;
			bucket_index = (bucket_index + 1) & buckets_mask;
		}
	}

	// Insert a bucket at this index and move the existing buckets to the right.
	void InsertBucket(Bucket inBucket, int inIndex)
	{
		Bucket    bucket       = inBucket;
		int       bucket_index = inIndex;
		const int buckets_mask = GetBucketSizeMask();
		while (true)
		{
			// Add it at the right index by swapping with existing bucket.
			gSwap(mBuckets[bucket_index], bucket);

			// If the existing bucket was empty, nothing else to do.
			if (bucket.mDistanceAndFingerprint == 0)
				break;

			// Otherwise keep swapping with the next bucket until an empty one is found.
			bucket.mDistanceAndFingerprint += Bucket::cDistanceIncrement;
			bucket_index = (bucket_index + 1) & buckets_mask;
		}
	}

	// Erase the bucket at this index and move the following buckets to the left if needed.
	void EraseBucket(int inIndex)
	{
		int       bucket_index = inIndex;
		const int buckets_mask = GetBucketSizeMask();

		while (true)
		{
			int    next_bucket_index = (bucket_index + 1) & buckets_mask;
			Bucket next_bucket       = mBuckets[next_bucket_index];

			// The next bucket only needs to be moved if has a distance >= 2 (ie. it's not in its ideal position).
			// Note: empty buckets have a distance of 0, so they also break the loop here.
			if (next_bucket.mDistanceAndFingerprint < 2 * Bucket::cDistanceIncrement)
				break;

			// Decrement the distance of the bucket, and move it to the previous index.
			next_bucket.mDistanceAndFingerprint -= Bucket::cDistanceIncrement;
			mBuckets[bucket_index] = next_bucket;

			bucket_index = next_bucket_index;
		}

		// Last bucket becomes empty.
		mBuckets[bucket_index] = {};
	}

	using KeyValueVector = Vector<KeyValue, taAllocator<KeyValue>>;
	using BucketVector = Vector<Bucket, taAllocator<Bucket>>;

	KeyValueVector	mKeyValues;		// Key-value pairs stored in a dense array.
	BucketVector	mBuckets;		// Bucket metadata.
};


namespace Details
{
	// VMem AreanaAllocator Alias with a single template param, to use with VMemHashMap.
	template <class taType>
	using VMemHashMapArenaAllocator = ArenaAllocator<taType, VMemArena<0>>;
}


// Alias for a HashMap using the TempAllocator.
// Resize without moving the Key/Values as long as it's the last Temp allocation (still needs a rehash). Allocates from the heap as a fallback.
template <
	typename taKey,
	typename taValue,
	typename taHash = Hash<taKey>
>
using TempHashMap = HashMap<taKey, taValue, taHash, TempAllocator>;

// HashMap variant using the VMemAllocator.
// It allocates virtual memory to grow while keepting the Key/Values at the same address.
// This is meant for very large HashMaps. Virtual memory operations are more expensive than small heap allocations.
template < 
	typename taKey,
	typename taValue,
	typename taHash = Hash<taKey>
>
struct VMemHashMap : HashMap<taKey, taValue, taHash, Details::VMemHashMapArenaAllocator>
{
	VMemHashMap()
	{
		mKeyValues = KeyValueVector(mVMemArena);
		mBuckets   = BucketVector(mVMemArena);
	}

	VMemHashMap(const VMemHashMap& inOther)
		: VMemHashMap() // Setup the allocator first.
	{
		*this = inOther;
	}

	VMemHashMap(VMemArena<0>&& ioMemArena)
		: mVMemArena(gMove(ioMemArena))
	{
		mKeyValues = KeyValueVector(mVMemArena);
		mBuckets   = BucketVector(mVMemArena);
	}

	~VMemHashMap()
	{
		// Clear the vectors manually first because they'll be destroyed after the VMemArena.
		mBuckets.ClearAndFreeMemory();
		mKeyValues.ClearAndFreeMemory();
	}

	// Move not allowed for now (could be implemented, but moving the arena itself is annoying).
	VMemHashMap(VMemHashMap&&) = delete;
	VMemHashMap& operator=(VMemHashMap&& ioOther) = delete;

private:
	using Base = HashMap<taKey, taValue, taHash, Details::VMemHashMapArenaAllocator>;
	using typename Base::KeyValueVector;
	using typename Base::BucketVector;
	using Base::mKeyValues;
	using Base::mBuckets;
	VMemArena<0> mVMemArena;
};


// HashSet variant of the HashMap (no values).
template <
	typename taKey,
	typename taHash = Hash<taKey>,
	template <typename> typename taAllocator = DefaultAllocator
>
using HashSet = HashMap<taKey, void, taHash, taAllocator>;


// Alias for a HashSet using the TempAllocator.
// Resize without moving the Keys as long as it's the last Temp allocation (still needs a rehash). Allocates from the heap as a fallback.
template <
	typename taKey,
	typename taHash = Hash<taKey>
>
using TempHashSet = HashSet<taKey, taHash, TempAllocator>;


// Alias for a HashSet using the VMemAllocator.
// It allocates virtual memory to grow while keepting the Keys at the same address.
// This is meant for very large HashSets. Virtual memory operations are more expensive than small heap allocations.
template <
	typename taKey,
	typename taHash = Hash<taKey>
>
using VMemHashSet = VMemHashMap<taKey, void, taHash>;


template <typename taKey, typename taValue, typename taHash, template <typename> class taAllocator>
HashMap<taKey, taValue, taHash, taAllocator>::HashMap(const HashMap& inOther)
{
	*this = inOther;
}


template <typename taKey, typename taValue, typename taHash, template <typename> class taAllocator>
HashMap<taKey, taValue, taHash, taAllocator>& HashMap<taKey, taValue, taHash, taAllocator>::operator=(
	const HashMap& inOther)
{
	Clear();
		
	mKeyValues.Reserve(inOther.mKeyValues.Capacity());
	mBuckets.Reserve(inOther.mBuckets.Capacity());

	mKeyValues = inOther.mKeyValues;
	mBuckets   = inOther.mBuckets;

	return *this;
}


template <typename taKey, typename taValue, typename taHash, template <typename> class taAllocator>
void HashMap<taKey, taValue, taHash, taAllocator>::Clear()
{
	mKeyValues.Clear();
	mBuckets.Clear();
	mBuckets.Resize(mBuckets.Capacity());
}


