// Copyright 2018 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// -----------------------------------------------------------------------------
// File: parallel_flat_hash_set.h
// -----------------------------------------------------------------------------
//
// An `absl::parallel_flat_hash_set<T>` is an unordered associative container designed to
// be a more efficient replacement for `std::unordered_set`. Like
// `unordered_set`, search, insertion, and deletion of set elements can be done
// as an `O(1)` operation. However, `parallel_flat_hash_set` (and other unordered
// associative containers known as the collection of Abseil "Swiss tables")
// contain other optimizations that result in both memory and computation
// advantages.
//
#ifndef ABSL_CONTAINER_PARALLEL_FLAT_HASH_SET_H_
#define ABSL_CONTAINER_PARALLEL_FLAT_HASH_SET_H_

#include <type_traits>
#include <utility>

#include "absl/algorithm/container.h"
#include "absl/base/macros.h"
#include "absl/container/internal/container_memory.h"
#include "absl/container/internal/hash_function_defaults.h"  // IWYU pragma: export
#include "absl/container/flat_hash_set.h"
#include "absl/container/internal/parallel_hash_set.h"  // IWYU pragma: export
#include "absl/memory/memory.h"

namespace absl {

// -----------------------------------------------------------------------------
// absl::parallel_flat_hash_set
// -----------------------------------------------------------------------------
//
// An `absl::parallel_flat_hash_set<T>` is an unordered associative container which has
// been optimized for both speed and memory footprint in most common use cases.
// Its interface is similar to that of `std::unordered_set<T>` with the
// following notable differences:
//
// * Requires keys that are CopyConstructible
// * Supports heterogeneous lookup, through `find()`, `operator[]()` and
//   `insert()`, provided that the set is provided a compatible heterogeneous
//   hashing function and equality operator.
// * Invalidates any references and pointers to elements within the table after
//   `rehash()`.
// * Contains a `capacity()` member function indicating the number of element
//   slots (open, deleted, and empty) within the hash set.
// * Returns `void` from the `erase(iterator)` overload.
//
// By default, `parallel_flat_hash_set` uses the `absl::Hash` hashing framework. All
// fundamental and Abseil types that support the `absl::Hash` framework have a
// compatible equality operator for comparing insertions into `parallel_flat_hash_map`.
// If your type is not yet supported by the `absl::Hash` framework, see
// absl/hash/hash.h for information on extending Abseil hashing to user-defined
// types.
//
// NOTE: A `parallel_flat_hash_set` stores its keys directly inside its implementation
// array to avoid memory indirection. Because a `parallel_flat_hash_set` is designed to
// move data when rehashed, set keys will not retain pointer stability. If you
// require pointer stability, consider using
// `absl::parallel_flat_hash_set<std::unique_ptr<T>>`. If your type is not moveable and
// you require pointer stability, consider `absl::node_hash_set` instead.
//
// Example:
//
//   // Create a flat hash set of three strings
//   absl::parallel_flat_hash_set<std::string> ducks =
//     {"huey", "dewey", "louie"};
//
//  // Insert a new element into the flat hash set
//  ducks.insert("donald");
//
//  // Force a rehash of the flat hash set
//  ducks.rehash(0);
//
//  // See if "dewey" is present
//  if (ducks.contains("dewey")) {
//    std::cout << "We found dewey!" << std::endl;
//  }
template <class T,
          class Hash      = absl::container_internal::hash_default_hash<T>,
          class Eq        = absl::container_internal::hash_default_eq<T>,
          class Allocator = std::allocator<T>,
          size_t N        = 4,
          class Mutex     = absl::NullMutex>
class parallel_flat_hash_set
    : public absl::container_internal::parallel_hash_set<
         N, absl::container_internal::raw_hash_set, Mutex,
         absl::container_internal::FlatHashSetPolicy<T>, 
         Hash, Eq, Allocator> {
  using Base = typename parallel_flat_hash_set::parallel_hash_set;

 public:
  // Constructors and Assignment Operators
  //
  // A parallel_flat_hash_set supports the same overload set as `std::unordered_map`
  // for construction and assignment:
  //
  // *  Default constructor
  //
  //    // No allocation for the table's elements is made.
  //    absl::parallel_flat_hash_set<std::string> set1;
  //
  // * Initializer List constructor
  //
  //   absl::parallel_flat_hash_set<std::string> set2 =
  //       {{"huey"}, {"dewey"}, {"louie"},};
  //
  // * Copy constructor
  //
  //   absl::parallel_flat_hash_set<std::string> set3(set2);
  //
  // * Copy assignment operator
  //
  //  // Hash functor and Comparator are copied as well
  //  absl::parallel_flat_hash_set<std::string> set4;
  //  set4 = set3;
  //
  // * Move constructor
  //
  //   // Move is guaranteed efficient
  //   absl::parallel_flat_hash_set<std::string> set5(std::move(set4));
  //
  // * Move assignment operator
  //
  //   // May be efficient if allocators are compatible
  //   absl::parallel_flat_hash_set<std::string> set6;
  //   set6 = std::move(set5);
  //
  // * Range constructor
  //
  //   std::vector<std::string> v = {"a", "b"};
  //   absl::parallel_flat_hash_set<std::string> set7(v.begin(), v.end());
  parallel_flat_hash_set() {}
  using Base::Base;

  // get the index of the the internal hash table used for a specific hash
  // size_t subidx(size_t hashval);
  //
  using Base::subidx;

  // get the number of internal hash tables used
  // size_t subcnt();
  //
  using Base::subcnt;

  // parallel_flat_hash_set::begin()
  //
  // Returns an iterator to the beginning of the `parallel_flat_hash_set`.
  using Base::begin;

  // parallel_flat_hash_set::cbegin()
  //
  // Returns a const iterator to the beginning of the `parallel_flat_hash_set`.
  using Base::cbegin;

  // parallel_flat_hash_set::cend()
  //
  // Returns a const iterator to the end of the `parallel_flat_hash_set`.
  using Base::cend;

  // parallel_flat_hash_set::end()
  //
  // Returns an iterator to the end of the `parallel_flat_hash_set`.
  using Base::end;

  // parallel_flat_hash_set::capacity()
  //
  // Returns the number of element slots (assigned, deleted, and empty)
  // available within the `parallel_flat_hash_set`.
  //
  // NOTE: this member function is particular to `absl::parallel_flat_hash_set` and is
  // not provided in the `std::unordered_map` API.
  using Base::capacity;

  // parallel_flat_hash_set::empty()
  //
  // Returns whether or not the `parallel_flat_hash_set` is empty.
  using Base::empty;

  // parallel_flat_hash_set::max_size()
  //
  // Returns the largest theoretical possible number of elements within a
  // `parallel_flat_hash_set` under current memory constraints. This value can be thought
  // of the largest value of `std::distance(begin(), end())` for a
  // `parallel_flat_hash_set<T>`.
  using Base::max_size;

  // parallel_flat_hash_set::size()
  //
  // Returns the number of elements currently within the `parallel_flat_hash_set`.
  using Base::size;

  // parallel_flat_hash_set::clear()
  //
  // Removes all elements from the `parallel_flat_hash_set`. Invalidates any references,
  // pointers, or iterators referring to contained elements.
  //
  // NOTE: this operation may shrink the underlying buffer. To avoid shrinking
  // the underlying buffer call `erase(begin(), end())`.
  using Base::clear;

  // parallel_flat_hash_set::erase()
  //
  // Erases elements within the `parallel_flat_hash_set`. Erasing does not trigger a
  // rehash. Overloads are listed below.
  //
  // void erase(const_iterator pos):
  //
  //   Erases the element at `position` of the `parallel_flat_hash_set`, returning
  //   `void`.
  //
  //   NOTE: this return behavior is different than that of STL containers in
  //   general and `std::unordered_map` in particular.
  //
  // iterator erase(const_iterator first, const_iterator last):
  //
  //   Erases the elements in the open interval [`first`, `last`), returning an
  //   iterator pointing to `last`.
  //
  // size_type erase(const key_type& key):
  //
  //   Erases the element with the matching key, if it exists.
  using Base::erase;

  // parallel_flat_hash_set::insert()
  //
  // Inserts an element of the specified value into the `parallel_flat_hash_set`,
  // returning an iterator pointing to the newly inserted element, provided that
  // an element with the given key does not already exist. If rehashing occurs
  // due to the insertion, all iterators are invalidated. Overloads are listed
  // below.
  //
  // std::pair<iterator,bool> insert(const T& value):
  //
  //   Inserts a value into the `parallel_flat_hash_set`. Returns a pair consisting of an
  //   iterator to the inserted element (or to the element that prevented the
  //   insertion) and a bool denoting whether the insertion took place.
  //
  // std::pair<iterator,bool> insert(T&& value):
  //
  //   Inserts a moveable value into the `parallel_flat_hash_set`. Returns a pair
  //   consisting of an iterator to the inserted element (or to the element that
  //   prevented the insertion) and a bool denoting whether the insertion took
  //   place.
  //
  // iterator insert(const_iterator hint, const T& value):
  // iterator insert(const_iterator hint, T&& value):
  //
  //   Inserts a value, using the position of `hint` as a non-binding suggestion
  //   for where to begin the insertion search. Returns an iterator to the
  //   inserted element, or to the existing element that prevented the
  //   insertion.
  //
  // void insert(InputIterator first, InputIterator last):
  //
  //   Inserts a range of values [`first`, `last`).
  //
  //   NOTE: Although the STL does not specify which element may be inserted if
  //   multiple keys compare equivalently, for `parallel_flat_hash_set` we guarantee the
  //   first match is inserted.
  //
  // void insert(std::initializer_list<T> ilist):
  //
  //   Inserts the elements within the initializer list `ilist`.
  //
  //   NOTE: Although the STL does not specify which element may be inserted if
  //   multiple keys compare equivalently within the initializer list, for
  //   `parallel_flat_hash_set` we guarantee the first match is inserted.
  using Base::insert;

  // parallel_flat_hash_set::emplace()
  //
  // Inserts an element of the specified value by constructing it in-place
  // within the `parallel_flat_hash_set`, provided that no element with the given key
  // already exists.
  //
  // The element may be constructed even if there already is an element with the
  // key in the container, in which case the newly constructed element will be
  // destroyed immediately.
  //
  // If rehashing occurs due to the insertion, all iterators are invalidated.
  using Base::emplace;

  // parallel_flat_hash_set::emplace_hint()
  //
  // Inserts an element of the specified value by constructing it in-place
  // within the `parallel_flat_hash_set`, using the position of `hint` as a non-binding
  // suggestion for where to begin the insertion search, and only inserts
  // provided that no element with the given key already exists.
  //
  // The element may be constructed even if there already is an element with the
  // key in the container, in which case the newly constructed element will be
  // destroyed immediately.
  //
  // If rehashing occurs due to the insertion, all iterators are invalidated.
  using Base::emplace_hint;

  // parallel_flat_hash_set::extract()
  //
  // Extracts the indicated element, erasing it in the process, and returns it
  // as a C++17-compatible node handle. Overloads are listed below.
  //
  // node_type extract(const_iterator position):
  //
  //   Extracts the element at the indicated position and returns a node handle
  //   owning that extracted data.
  //
  // node_type extract(const key_type& x):
  //
  //   Extracts the element with the key matching the passed key value and
  //   returns a node handle owning that extracted data. If the `parallel_flat_hash_set`
  //   does not contain an element with a matching key, this function returns an
  //   empty node handle.
  using Base::extract;

  // parallel_flat_hash_set::merge()
  //
  // Extracts elements from a given `source` flat hash map into this
  // `parallel_flat_hash_set`. If the destination `parallel_flat_hash_set` already contains an
  // element with an equivalent key, that element is not extracted.
  using Base::merge;

  // parallel_flat_hash_set::swap(parallel_flat_hash_set& other)
  //
  // Exchanges the contents of this `parallel_flat_hash_set` with those of the `other`
  // flat hash map, avoiding invocation of any move, copy, or swap operations on
  // individual elements.
  //
  // All iterators and references on the `parallel_flat_hash_set` remain valid, excepting
  // for the past-the-end iterator, which is invalidated.
  //
  // `swap()` requires that the flat hash set's hashing and key equivalence
  // functions be Swappable, and are exchaged using unqualified calls to
  // non-member `swap()`. If the map's allocator has
  // `std::allocator_traits<allocator_type>::propagate_on_container_swap::value`
  // set to `true`, the allocators are also exchanged using an unqualified call
  // to non-member `swap()`; otherwise, the allocators are not swapped.
  using Base::swap;

  // parallel_flat_hash_set::rehash(count)
  //
  // Rehashes the `parallel_flat_hash_set`, setting the number of slots to be at least
  // the passed value. If the new number of slots increases the load factor more
  // than the current maximum load factor
  // (`count` < `size()` / `max_load_factor()`), then the new number of slots
  // will be at least `size()` / `max_load_factor()`.
  //
  // To force a rehash, pass rehash(0).
  //
  // NOTE: unlike behavior in `std::unordered_set`, references are also
  // invalidated upon a `rehash()`.
  using Base::rehash;

  // parallel_flat_hash_set::reserve(count)
  //
  // Sets the number of slots in the `parallel_flat_hash_set` to the number needed to
  // accommodate at least `count` total elements without exceeding the current
  // maximum load factor, and may rehash the container if needed.
  using Base::reserve;

  // parallel_flat_hash_set::contains()
  //
  // Determines whether an element comparing equal to the given `key` exists
  // within the `parallel_flat_hash_set`, returning `true` if so or `false` otherwise.
  using Base::contains;

  // parallel_flat_hash_set::count(const Key& key) const
  //
  // Returns the number of elements comparing equal to the given `key` within
  // the `parallel_flat_hash_set`. note that this function will return either `1` or `0`
  // since duplicate elements are not allowed within a `parallel_flat_hash_set`.
  using Base::count;

  // parallel_flat_hash_set::equal_range()
  //
  // Returns a closed range [first, last], defined by a `std::pair` of two
  // iterators, containing all elements with the passed key in the
  // `parallel_flat_hash_set`.
  using Base::equal_range;

  // parallel_flat_hash_set::find()
  //
  // Finds an element with the passed `key` within the `parallel_flat_hash_set`.
  using Base::find;

  // parallel_flat_hash_set::bucket_count()
  //
  // Returns the number of "buckets" within the `parallel_flat_hash_set`. Note that
  // because a flat hash map contains all elements within its internal storage,
  // this value simply equals the current capacity of the `parallel_flat_hash_set`.
  using Base::bucket_count;

  // parallel_flat_hash_set::load_factor()
  //
  // Returns the current load factor of the `parallel_flat_hash_set` (the average number
  // of slots occupied with a value within the hash map).
  using Base::load_factor;

  // parallel_flat_hash_set::max_load_factor()
  //
  // Manages the maximum load factor of the `parallel_flat_hash_set`. Overloads are
  // listed below.
  //
  // float parallel_flat_hash_set::max_load_factor()
  //
  //   Returns the current maximum load factor of the `parallel_flat_hash_set`.
  //
  // void parallel_flat_hash_set::max_load_factor(float ml)
  //
  //   Sets the maximum load factor of the `parallel_flat_hash_set` to the passed value.
  //
  //   NOTE: This overload is provided only for API compatibility with the STL;
  //   `parallel_flat_hash_set` will ignore any set load factor and manage its rehashing
  //   internally as an implementation detail.
  using Base::max_load_factor;

  // parallel_flat_hash_set::get_allocator()
  //
  // Returns the allocator function associated with this `parallel_flat_hash_set`.
  using Base::get_allocator;

  // parallel_flat_hash_set::hash_function()
  //
  // Returns the hashing function used to hash the keys within this
  // `parallel_flat_hash_set`.
  using Base::hash_function;

  // parallel_flat_hash_set::key_eq()
  //
  // Returns the function used for comparing keys equality.
  using Base::key_eq;
};

namespace container_algorithm_internal {

// Specialization of trait in absl/algorithm/container.h
    template <class T, class Hash, class Eq, class Allocator, size_t N, class Mutex>
    struct IsUnorderedContainer<absl::parallel_flat_hash_set<T, Hash, Eq, Allocator, N, Mutex>>
    : std::true_type {};

}  // namespace container_algorithm_internal

}  // namespace absl

#endif  // ABSL_CONTAINER_PARALLEL_FLAT_HASH_SET_H_
