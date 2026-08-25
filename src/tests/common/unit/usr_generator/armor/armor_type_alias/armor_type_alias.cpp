// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// =============================================================================
// 1. PRIMITIVE TYPE ALIASES
// =============================================================================

using Byte    = unsigned char;
using Int8    = signed char;
using Int16   = short;
using Int32   = int;
using Int64   = long long;
using UInt8   = unsigned char;
using UInt16  = unsigned short;
using UInt32  = unsigned int;
using UInt64  = unsigned long long;
using Float32 = float;
using Float64 = double;
using Char    = char;
using Bool    = bool;

// =============================================================================
// 2. CHAINED PRIMITIVE ALIASES  (alias of alias of alias)
// =============================================================================

using Size      = UInt64;           // chain depth 1
using Index     = Size;             // chain depth 2
using Offset    = Index;            // chain depth 3
using Handle    = UInt32;           // chain depth 1
using SubHandle = Handle;           // chain depth 2
using Token     = SubHandle;        // chain depth 3

// =============================================================================
// 3. POINTER & REFERENCE ALIASES
// =============================================================================

using BytePtr      = Byte *;
using ConstBytePtr = const Byte *;
using Int32Ptr     = Int32 *;
using Int32Ref     = Int32 &;       // reference alias
using VoidPtr      = void *;
using ConstVoidPtr = const void *;

// Chained pointer aliases
using RawBuffer     = BytePtr;          // alias of pointer alias
using ConstBuffer   = ConstBytePtr;     // alias of const-pointer alias
using BufferHandle  = RawBuffer;        // alias of alias of pointer alias

// =============================================================================
// 4. STD CONTAINER ALIASES  (single-level)
// =============================================================================

using IntVec        = std::vector<int>;
using FloatVec      = std::vector<float>;
using DoubleVec     = std::vector<double>;
using StringVec     = std::vector<std::string>;
using ByteVec       = std::vector<Byte>;
using IntIntMap     = std::map<int, int>;
using StringIntMap  = std::map<std::string, int>;
using IntStringMap  = std::map<int, std::string>;
using StringStrMap  = std::map<std::string, std::string>;
using IntIntUMap    = std::unordered_map<int, int>;
using StrIntUMap    = std::unordered_map<std::string, int>;
using IntPair       = std::pair<int, int>;
using StrIntPair    = std::pair<std::string, int>;
using IntTriple     = std::tuple<int, int, int>;
using MixedTuple    = std::tuple<int, float, std::string>;
using IntArr4       = std::array<int, 4>;
using FloatArr3     = std::array<float, 3>;

// =============================================================================
// 5. CHAINED CONTAINER ALIASES  (alias of alias)
// =============================================================================

using Row           = IntVec;               // chain depth 1
using Matrix        = std::vector<Row>;     // container of aliased type
using Cube          = std::vector<Matrix>;  // container of container alias

using Record        = StringIntMap;         // chain depth 1
using RecordSet     = std::vector<Record>;  // container of aliased map
using RecordIndex   = std::map<std::string, RecordSet>; // map of aliased vector

using Pair          = IntPair;              // chain depth 1
using PairVec       = std::vector<Pair>;    // container of aliased pair
using PairMap       = std::map<int, Pair>;  // map with aliased value

// =============================================================================
// 6. NESTED CONTAINER ALIASES  (containers of containers)
// =============================================================================

using VecOfVecInt       = std::vector<std::vector<int>>;
using VecOfVecFloat     = std::vector<std::vector<float>>;
using MapOfVecInt       = std::map<std::string, std::vector<int>>;
using MapOfMapInt       = std::map<std::string, std::map<std::string, int>>;
using VecOfMapStrInt    = std::vector<std::map<std::string, int>>;
using UMapOfVecStr      = std::unordered_map<int, std::vector<std::string>>;
using VecOfPair         = std::vector<std::pair<int, std::string>>;
using MapOfTuple        = std::map<int, std::tuple<int, float, std::string>>;

// Chained nested container aliases
using Grid              = VecOfVecInt;              // alias of nested container
using GridRow           = IntVec;                   // alias of inner container
using SparseGrid        = MapOfVecInt;              // alias of map-of-vec
using SparseGridIndex   = std::map<int, SparseGrid>;// map of aliased map-of-vec

// =============================================================================
// 7. SMART POINTER ALIASES
// =============================================================================

using IntShared     = std::shared_ptr<int>;
using IntUnique     = std::unique_ptr<int>;
using IntWeak       = std::weak_ptr<int>;
using VoidShared    = std::shared_ptr<void>;

// Chained smart pointer aliases
using SharedInt     = IntShared;                    // chain depth 1
using SharedIntVec  = std::vector<SharedInt>;       // container of aliased smart ptr
using SharedIntMap  = std::map<int, SharedInt>;     // map of aliased smart ptr

// =============================================================================
// 8. FUNCTION / CALLABLE ALIASES
// =============================================================================

using VoidFn            = std::function<void()>;
using IntFn             = std::function<int()>;
using IntIntFn          = std::function<int(int)>;
using IntIntIntFn       = std::function<int(int, int)>;
using BoolIntFn         = std::function<bool(int)>;
using VoidIntFn         = std::function<void(int)>;
using VoidStrFn         = std::function<void(std::string)>;
using StrStrFn          = std::function<std::string(std::string)>;

// Function aliases using aliased types
using SizeFn            = std::function<Size()>;            // uses primitive alias
using IndexMapFn        = std::function<Index(Index)>;      // uses chained alias
using RowTransformFn    = std::function<Row(Row)>;          // uses container alias
using MatrixFn          = std::function<Matrix(Matrix)>;    // uses chained container alias

// Chained function aliases
using Callback          = VoidFn;                           // chain depth 1
using EventHandler      = Callback;                         // chain depth 2
using SignalHandler     = EventHandler;                     // chain depth 3

using Predicate         = BoolIntFn;                        // chain depth 1
using Filter            = Predicate;                        // chain depth 2

using Transformer       = IntIntFn;                         // chain depth 1
using Mapper            = Transformer;                      // chain depth 2
using Reducer           = Mapper;                           // chain depth 3

// =============================================================================
// 9. TEMPLATE TYPE ALIAS  (alias templates)
// =============================================================================

template <typename T>
using Vec = std::vector<T>;

template <typename T>
using Ptr = std::shared_ptr<T>;

template <typename T>
using UniquePtr = std::unique_ptr<T>;

template <typename K, typename V>
using Dict = std::map<K, V>;

template <typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template <typename T>
using Opt = std::pair<T, bool>;    // lightweight optional-like

template <typename T>
using Grid2D = std::vector<std::vector<T>>;

template <typename T>
using Grid3D = std::vector<Grid2D<T>>;  // chained template alias

template <typename K, typename V>
using MultiDict = std::map<K, std::vector<V>>;

template <typename T>
using Fn = std::function<T>;       // generic callable alias

template <typename T>
using PtrVec = Vec<Ptr<T>>;        // chained template aliases

template <typename K, typename V>
using PtrDict = Dict<K, Ptr<V>>;   // chained template aliases

// =============================================================================
// 10. COMBINATIONS: MIXING PRIMITIVE, CONTAINER, SMART-PTR, FUNCTION ALIASES
// =============================================================================

// Primitive alias inside containers
using IndexVec          = Vec<Index>;               // template alias + chained primitive
using HandleMap         = Dict<Handle, SubHandle>;  // template alias + chained primitives
using TokenSet          = std::vector<Token>;       // container of depth-3 chained alias

// Smart pointer of aliased container
using SharedRow         = Ptr<Row>;                 // smart ptr of container alias
using SharedMatrix      = Ptr<Matrix>;              // smart ptr of chained container alias
using UniqueGrid        = UniquePtr<Grid>;          // unique ptr of aliased nested container

// Container of smart pointers of aliased types
using SharedRowVec      = Vec<SharedRow>;           // vec of (shared ptr of alias)
using SharedMatrixVec   = Vec<SharedMatrix>;        // vec of (shared ptr of chained alias)

// Function aliases using template aliases
using IndexTransform    = Fn<Index(Index)>;         // template fn alias + chained primitive
using RowFn             = Fn<Row(Row)>;             // template fn alias + container alias
using MatrixBuilder     = Fn<Matrix()>;             // template fn alias + chained container
using GridReducer       = Fn<Grid(Grid, Grid)>;     // template fn alias + nested alias

// Map of function aliases
using CallbackMap       = Dict<std::string, Callback>;      // map of chained fn alias
using HandlerRegistry   = HashMap<std::string, EventHandler>; // hashmap of depth-2 fn alias

// Tuple combining aliased types
using RecordTuple       = std::tuple<Index, Handle, Row>;   // tuple of mixed aliases
using FullRecord        = std::tuple<Token, SharedRow, Callback>; // tuple of depth-3 aliases

// =============================================================================
// 11. DEEP CHAINING  (5+ levels)
// =============================================================================

// Primitive chain
using L1 = int;
using L2 = L1;
using L3 = L2;
using L4 = L3;
using L5 = L4;
using L6 = L5;

// Container chain
using C1 = std::vector<int>;
using C2 = C1;
using C3 = C2;
using C4 = C3;
using C5 = C4;

// Function chain
using F1 = std::function<void(int)>;
using F2 = F1;
using F3 = F2;
using F4 = F3;
using F5 = F4;

// Smart pointer chain
using S1 = std::shared_ptr<int>;
using S2 = S1;
using S3 = S2;
using S4 = S3;
using S5 = S4;

// Cross-kind chaining: primitive -> container -> smart ptr -> function
using XBase     = UInt32;                           // primitive alias
using XVec      = std::vector<XBase>;               // container of primitive alias
using XShared   = std::shared_ptr<XVec>;            // smart ptr of container alias
using XFn       = std::function<XShared(XBase)>;    // function using smart ptr + primitive alias
using XCallback = XFn;                              // alias of cross-kind chain
using XHandler  = XCallback;                        // alias of alias of cross-kind chain
