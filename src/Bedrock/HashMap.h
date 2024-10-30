// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Bedrock/Core.h>
#include <Bedrock/Hash.h>
#include <Bedrock/Vector.h>


enum class EInsertResult : int8
{
	Found,
	Added,
	Replaced
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
struct HashMap
{
	struct KeyValue
	{
		taKey	mKey;
		taValue mValue;
	};

	struct InsertResult
	{
		const taKey&  mKey;
		taValue&      mValue;
		EInsertResult mResult;
	};

	using ConstIter = const KeyValue*;
	using Iter = const KeyValue*;

	bool Empty() const
	{
		return mKeyValues.Empty();
	}

	ConstIter Begin() const { return mKeyValues.Begin(); }
	ConstIter End() const { return mKeyValues.Begin(); }
	Iter Begin() { return mKeyValues.Begin(); }
	Iter End() { return mKeyValues.Begin(); }

	bool IsFull() const
	{
		// todo
		return false;
	}

	void Grow()
	{
		// todo
	}

	Iter Find(const taKey& inKey)
	{
		if (Empty())
			return End();

		// Try to find the key.
		auto [bucket_index, _, found] = FindInternal(inKey);

		// If it was found, return an iterator.
		if (found)
			return mKeyValues.Begin() + mBuckets[bucket_index].mKeyValueIndex;

		// Otherwise return End.
		return End();
	}


	InsertResult Insert(const taKey& inKey, const taValue& inValue)
	{
		if (IsFull())
			Grow();

		// Try to find the key.
		auto [bucket_index, distance_and_fingerprint, found] = FindInternal(inKey);

		if (found)
		{
			// Key already exist.
			KeyValue& key_value = mKeyValues[mBuckets[bucket_index].mKeyValueIndex];
			return { key_value.mKey, key_value.mValue, EInsertResult::Found };
		}

		// Key does not exist, add it.
		mKeyValues.EmplaceBack(inKey, inValue);

		// Prepare a new bucket for it.
		Bucket new_bucket = { distance_and_fingerprint, mKeyValues.Size() - 1 };

		const int buckets_mask = GetBucketSizeMask();
		while (true)
		{
			// Add it at the right index by swapping with existing bucket.
			gSwap(mBuckets[bucket_index], new_bucket);

			// If the existing bucket was empty, nothing else to do.
			if (new_bucket.mDistanceAndFingerprint == 0)
				break;

			// Otherwise keep swapping with the next bucket until an empty one is found.
			new_bucket.mDistanceAndFingerprint += Bucket::cDistanceIncrement;
			bucket_index = (bucket_index + 1) & buckets_mask;
		}

		KeyValue& key_value = mKeyValues.Back();
		return { key_value.mKey, key_value.mValue, EInsertResult::Added };
	}

	bool Erase(const taKey& inKey)
	{
		if (Empty())
			return false;

		// Try to find the key.
		auto [bucket_index, distance_and_fingerprint, found] = FindInternal(inKey);

		if (found == false)
			return false; // Key does not exist.

		// Swap the key-value to erase with the last one, to minimize the number of moves.
		int key_value_index_to_erase = mBuckets[bucket_index].mKeyValueIndex;
		int key_value_index_swapped  = mBuckets.Back().mKeyValueIndex;
		mKeyValues.SwapErase(key_value_index_to_erase);

		// If the erased key was actually the last one, nothing else to do.
		if (key_value_index_to_erase == mKeyValues.Size())
			return true;

		// Otherwise we need to find the bucket of the key we moved to update its index.
		const uint64 hash         = gHash(mKeyValues.Back().mKey);
		const int    buckets_mask = GetBucketSizeMask();
		bucket_index              = (int)hash & buckets_mask;

		// No need to compare fingerprints and keys, it's faster to just compare the key-value index. We know it will be found.
		while (true)
		{
			Bucket& bucket = mBuckets[bucket_index];
			if (bucket.mKeyValueIndex == key_value_index_swapped)
			{
				// Found it, update the index.
				bucket.mKeyValueIndex = key_value_index_to_erase;
				break;
			}

			// Go to the next bucket.
			bucket_index = (bucket_index + 1) & buckets_mask;
		}

		return true;
	}


	HashMap()
	{
		static_assert(cIsTriviallyDefaultConstructible<Bucket>);
		mBuckets.Resize(16);
	}

private:

	int GetBucketSizeMask() const
	{
		gAssert(!mBuckets.Empty());

		// The number of buckets is a power of 2, so we can use a bitwise and as a faster modulo.
		return mBuckets.Size() - 1;
	}

	struct FindResult
	{
		int    mBucketIndex;
		uint32 mDistanceAndFingerprint;
		bool   mFound;
	};

	FindResult FindInternal(const taKey& inKey) const
	{
		// Calculate the hash.
		const uint64 hash         = gHash(inKey);

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
			if (bucket.mDistanceAndFingerprint == distance_and_fingerprint) [[likely]]
			{
				// Then check if the key is equal too.
				if (mKeyValues[bucket.mKeyValueIndex].mKey == inKey) [[likely]]
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


	Vector<KeyValue> mKeyValues;	// Key-value pairs stored in a dense array.

	using Bucket = Details::HashMapBucket;
	Vector<Bucket> mBuckets;
	uint32         mSizeMask = 0;
};



