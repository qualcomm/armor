// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <array>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// =============================================================================
// 1. PRIMITIVE TYPE ALIASES
// =============================================================================

using Byte    = unsigned char;
using Int8    = signed char;
using Int16   = short;
using Int32   = long;               // CHANGED: int -> long
using Int64   = long long;
using UInt8   = unsigned char;
using UInt16  = unsigned short;
using UInt32  = unsigned int;
using UInt64  = unsigned long long;
using Float32 = double;             // CHANGED: float -> double
using Float64 = double;
using Char    = char;
// REMOVED: Bool = bool
using NativeInt = int;              // ADDED
using NativeLong = long;            // ADDED
using Byte16  = uint16_t;           // ADDED

// =============================================================================
// 2. CHAINED PRIMITIVE ALIASES  (alias of alias of alias)
// =============================================================================

using Size      = UInt32;           // CHANGED: UInt64 -> UInt32  (chain depth 1)
using Index     = Size;             // chain depth 2
using Offset    = Index;            // chain depth 3
using Stride    = Offset;           // ADDED: chain depth 4
// REMOVED: Handle = UInt32
// REMOVED: SubHandle = Handle
// REMOVED: Token = SubHandle
using NodeId    = UInt64;           // ADDED: new chain root
using EdgeId    = NodeId;           // ADDED: chain depth 2
using GraphId   = EdgeId;           // ADDED: chain depth 3

// =============================================================================
// 3. POINTER & REFERENCE ALIASES
// =============================================================================

using BytePtr      = Byte *;
using ConstBytePtr = const Byte *;
// REMOVED: Int32Ptr = Int32 *
// REMOVED: Int32Ref = Int32 &
using VoidPtr      = void *;
using ConstVoidPtr = const void *;
using Int64Ptr     = Int64 *;       // ADDED
using Int64Ref     = Int64 &;       // ADDED
using ByteRef      = Byte &;        // ADDED

// Chained pointer aliases
using RawBuffer     = BytePtr;          // alias of pointer alias (unchanged)
// REMOVED: ConstBuffer = ConstBytePtr
using BufferHandle  = RawBuffer;        // alias of alias of pointer alias (unchanged)
using SecureBuffer  = ConstBytePtr;     // ADDED: replaces ConstBuffer with new name
using MutableBuffer = BytePtr;          // ADDED

// =============================================================================
// 4. STD CONTAINER ALIASES  (single-level)
// =============================================================================

using IntVec        = std::vector<int>;
// REMOVED: FloatVec = std::vector<float>
using DoubleVec     = std::vector<double>;
using StringVec     = std::vector<std::string>;
using ByteVec       = std::vector<Byte>;
using IntIntMap     = std::map<int, int>;
using StringIntMap  = std::map<std::string, int>;
// REMOVED: IntStringMap = std::map<int, std::string>
using StringStrMap  = std::map<std::string, std::string>;
// REMOVED: IntIntUMap = std::unordered_map<int, int>
using StrIntUMap    = std::unordered_map<std::string, int>;
using IntPair       = std::pair<int, int>;
using StrIntPair    = std::pair<std::string, int>;
using IntTriple     = std::tuple<int, int, int>;
using MixedTuple    = std::tuple<int, double, std::string>; // CHANGED: float -> double
// REMOVED: IntArr4 = std::array<int, 4>
using FloatArr3     = std::array<float, 3>;
using FloatVec      = std::vector<double>;  // CHANGED: vector<float> -> vector<double>
using IntDeque      = std::deque<int>;      // ADDED
using StringList    = std::list<std::string>; // ADDED
using IntSet        = std::set<int>;          // ADDED
using StringUSet    = std::unordered_set<std::string>; // ADDED
using IntArr8       = std::array<int, 8>;     // ADDED

// =============================================================================
// 5. CHAINED CONTAINER ALIASES  (alias of alias)
// =============================================================================

using Row           = IntVec;               // chain depth 1 (unchanged)
using Matrix        = std::vector<Row>;     // container of aliased type (unchanged)
// REMOVED: Cube = std::vector<Matrix>
using Tensor        = std::vector<Matrix>;  // ADDED: replaces Cube with new name
using HyperTensor   = std::vector<Tensor>;  // ADDED: chain depth 4

using Record        = StringIntMap;         // chain depth 1 (unchanged)
using RecordSet     = std::vector<Record>;  // container of aliased map (unchanged)
// REMOVED: RecordIndex = std::map<std::string, RecordSet>
using RecordMap     = std::unordered_map<std::string, RecordSet>; // ADDED: unordered variant

// REMOVED: Pair = IntPair
// REMOVED: PairVec = std::vector<Pair>
// REMOVED: PairMap = std::map<int, Pair>
using TaggedPair    = std::pair<std::string, int>; // ADDED: new pair alias
using TaggedPairVec = std::vector<TaggedPair>;     // ADDED: container of new pair alias

// =============================================================================
// 6. NESTED CONTAINER ALIASES  (containers of containers)
// =============================================================================

using VecOfVecInt       = std::vector<std::vector<int>>;    // unchanged
// REMOVED: VecOfVecFloat = std::vector<std::vector<float>>
using VecOfVecDouble    = std::vector<std::vector<double>>; // ADDED: float -> double
using MapOfVecInt       = std::map<std::string, std::vector<int>>; // unchanged
// REMOVED: MapOfMapInt = std::map<std::string, std::map<std::string, int>>
using MapOfMapStr       = std::map<std::string, std::map<std::string, std::string>>; // ADDED: int -> string value
using VecOfMapStrInt    = std::vector<std::map<std::string, int>>; // unchanged
using UMapOfVecStr      = std::unordered_map<int, std::vector<std::string>>; // unchanged
// REMOVED: VecOfPair = std::vector<std::pair<int, std::string>>
using VecOfPairStrStr   = std::vector<std::pair<std::string, std::string>>; // ADDED: int key -> string key
using MapOfTuple        = std::map<int, std::tuple<int, double, std::string>>; // CHANGED: float -> double in tuple
using SetOfVecInt       = std::set<std::vector<int>>;       // ADDED
using UMapOfSet         = std::unordered_map<int, std::set<std::string>>; // ADDED

// Chained nested container aliases
using Grid              = VecOfVecInt;              // unchanged
// REMOVED: GridRow = IntVec
using GridCol           = std::vector<int>;         // ADDED: replaces GridRow
using SparseGrid        = MapOfVecInt;              // unchanged
// REMOVED: SparseGridIndex = std::map<int, SparseGrid>
using SparseGridMap     = std::unordered_map<int, SparseGrid>; // ADDED: map -> unordered_map

// =============================================================================
// 7. SMART POINTER ALIASES
// =============================================================================

using IntShared     = std::shared_ptr<int>;
// REMOVED: IntUnique = std::unique_ptr<int>
using IntWeak       = std::weak_ptr<int>;
// REMOVED: VoidShared = std::shared_ptr<void>
using Int64Shared   = std::shared_ptr<Int64>;       // ADDED
using Int64Unique   = std::unique_ptr<Int64>;       // ADDED
using FloatShared   = std::shared_ptr<float>;       // ADDED

// Chained smart pointer aliases
using SharedInt     = IntShared;                    // chain depth 1 (unchanged)
using SharedIntVec  = std::vector<SharedInt>;       // unchanged
// REMOVED: SharedIntMap = std::map<int, SharedInt>
using SharedIntUMap = std::unordered_map<int, SharedInt>; // ADDED: map -> unordered_map
using WeakIntVec    = std::vector<IntWeak>;         // ADDED

// =============================================================================
// 8. FUNCTION / CALLABLE ALIASES
// =============================================================================

using VoidFn            = std::function<void()>;
// REMOVED: IntFn = std::function<int()>
using IntIntFn          = std::function<int(int)>;
// REMOVED: IntIntIntFn = std::function<int(int, int)>
using BoolIntFn         = std::function<bool(int)>;
using VoidIntFn         = std::function<void(int)>;
// REMOVED: VoidStrFn = std::function<void(std::string)>
using StrStrFn          = std::function<std::string(std::string)>;
using LongFn            = std::function<long()>;    // ADDED
using BoolStrFn         = std::function<bool(std::string)>; // ADDED
using VoidVoidPtrFn     = std::function<void(void*)>; // ADDED

// Function aliases using aliased types
using SizeFn            = std::function<Size()>;            // unchanged (Size type changed)
using IndexMapFn        = std::function<Index(Index)>;      // unchanged
// REMOVED: RowTransformFn = std::function<Row(Row)>
using MatrixFn          = std::function<Matrix(Matrix)>;    // unchanged
using TensorFn          = std::function<Tensor(Tensor)>;    // ADDED: uses new Tensor alias
using NodeFn            = std::function<NodeId(NodeId)>;    // ADDED: uses new NodeId alias

// Chained function aliases
using Callback          = VoidFn;                           // chain depth 1 (unchanged)
using EventHandler      = Callback;                         // chain depth 2 (unchanged)
// REMOVED: SignalHandler = EventHandler
using Listener          = EventHandler;                     // ADDED: replaces SignalHandler
using Observer          = Listener;                         // ADDED: chain depth 4

// REMOVED: Predicate = BoolIntFn
// REMOVED: Filter = Predicate
using Condition         = BoolIntFn;                        // ADDED: replaces Predicate
using Guard             = Condition;                        // ADDED: replaces Filter

using Transformer       = IntIntFn;                         // chain depth 1 (unchanged)
// REMOVED: Mapper = Transformer
// REMOVED: Reducer = Mapper
using Converter         = Transformer;                      // ADDED: replaces Mapper
using Processor         = Converter;                        // ADDED: replaces Reducer

// =============================================================================
// 9. TEMPLATE TYPE ALIAS  (alias templates)
// =============================================================================

template <typename T>
using Vec = std::vector<T>;         // unchanged

template <typename T>
using Ptr = std::shared_ptr<T>;     // unchanged

template <typename T>
using UniquePtr = std::unique_ptr<T>; // unchanged

template <typename K, typename V>
using Dict = std::map<K, V>;        // unchanged

template <typename K, typename V>
using HashMap = std::unordered_map<K, V>; // unchanged

// REMOVED: Opt = std::pair<T, bool>
template <typename T>
using Optional = std::pair<T, bool>; // ADDED: renamed from Opt

template <typename T>
using Grid2D = std::vector<std::vector<T>>; // unchanged

// REMOVED: Grid3D = std::vector<Grid2D<T>>
template <typename T>
using Volume = std::vector<Grid2D<T>>; // ADDED: renamed from Grid3D

template <typename K, typename V>
using MultiDict = std::map<K, std::vector<V>>; // unchanged

template <typename T>
using Fn = std::function<T>;        // unchanged

template <typename T>
using PtrVec = Vec<Ptr<T>>;         // unchanged

template <typename K, typename V>
using PtrDict = Dict<K, Ptr<V>>;    // unchanged

template <typename T>               // ADDED
using WeakPtr = std::weak_ptr<T>;

template <typename T>               // ADDED
using Deque = std::deque<T>;

template <typename T>               // ADDED
using Set = std::set<T>;

template <typename K, typename V>   // ADDED
using MultiMap = std::multimap<K, V>;

// =============================================================================
// 10. COMBINATIONS: MIXING PRIMITIVE, CONTAINER, SMART-PTR, FUNCTION ALIASES
// =============================================================================

// Primitive alias inside containers
using IndexVec          = Vec<Index>;               // unchanged
// REMOVED: HandleMap = Dict<Handle, SubHandle>
using NodeMap           = Dict<NodeId, EdgeId>;     // ADDED: uses new NodeId/EdgeId aliases
// REMOVED: TokenSet = std::vector<Token>
using GraphIdVec        = std::vector<GraphId>;     // ADDED: uses new GraphId alias

// Smart pointer of aliased container
using SharedRow         = Ptr<Row>;                 // unchanged
// REMOVED: SharedMatrix = Ptr<Matrix>
using SharedTensor      = Ptr<Tensor>;              // ADDED: uses new Tensor alias
// REMOVED: UniqueGrid = UniquePtr<Grid>
using UniqueMatrix      = UniquePtr<Matrix>;        // ADDED: unique ptr of Matrix

// Container of smart pointers of aliased types
using SharedRowVec      = Vec<SharedRow>;           // unchanged
// REMOVED: SharedMatrixVec = Vec<SharedMatrix>
using SharedTensorVec   = Vec<SharedTensor>;        // ADDED: uses new SharedTensor
using WeakRowVec        = Vec<WeakPtr<Row>>;        // ADDED: weak ptr of Row

// Function aliases using template aliases
using IndexTransform    = Fn<Index(Index)>;         // unchanged
// REMOVED: RowFn = Fn<Row(Row)>
using RowProcessor      = Fn<Row(Row, Index)>;      // ADDED: extra Index param
using MatrixBuilder     = Fn<Matrix()>;             // unchanged
// REMOVED: GridReducer = Fn<Grid(Grid, Grid)>
using TensorReducer     = Fn<Tensor(Tensor, Tensor)>; // ADDED: uses Tensor alias

// Map of function aliases
using CallbackMap       = Dict<std::string, Callback>;      // unchanged
// REMOVED: HandlerRegistry = HashMap<std::string, EventHandler>
using ListenerRegistry  = HashMap<std::string, Listener>;   // ADDED: uses new Listener alias

// Tuple combining aliased types
// REMOVED: RecordTuple = std::tuple<Index, Handle, Row>
using NodeTuple         = std::tuple<NodeId, EdgeId, Row>;  // ADDED: uses new NodeId/EdgeId
using FullRecord        = std::tuple<GraphId, SharedRow, Callback>; // CHANGED: Token->GraphId, SharedRow unchanged

// =============================================================================
// 11. DEEP CHAINING  (5+ levels)
// =============================================================================

// Primitive chain
using L1 = int;
using L2 = L1;
using L3 = L2;
// REMOVED: L4 = L3
// REMOVED: L5 = L4
// REMOVED: L6 = L5
using L4 = long;                    // CHANGED: L3 -> long (breaks chain, new root type)
using L5 = L4;                      // chain continues from new L4

// Container chain
using C1 = std::vector<int>;
using C2 = C1;
using C3 = C2;
using C4 = C3;
// REMOVED: C5 = C4
using C5 = std::deque<int>;         // CHANGED: C4 (vector) -> deque (new root type)

// Function chain
using F1 = std::function<void(int)>;
using F2 = F1;
// REMOVED: F3 = F2
// REMOVED: F4 = F3
// REMOVED: F5 = F4
using F3 = std::function<void(long)>; // CHANGED: void(int) -> void(long)
using F4 = F3;                        // chain continues from new F3

// Smart pointer chain
// REMOVED: S1 = std::shared_ptr<int>
// REMOVED: S2 = S1
// REMOVED: S3 = S2
// REMOVED: S4 = S3
// REMOVED: S5 = S4
using S1 = std::shared_ptr<long>;   // CHANGED: shared_ptr<int> -> shared_ptr<long>
using S2 = S1;
using S3 = S2;

// Cross-kind chaining: primitive -> container -> smart ptr -> function
using XBase     = UInt64;                           // CHANGED: UInt32 -> UInt64
using XVec      = std::vector<XBase>;               // unchanged (type of XBase changed)
using XShared   = std::shared_ptr<XVec>;            // unchanged
// REMOVED: XFn = std::function<XShared(XBase)>
using XFn       = std::function<bool(XBase)>;       // CHANGED: XShared(XBase) -> bool(XBase)
using XCallback = XFn;                              // unchanged
// REMOVED: XHandler = XCallback
using XDispatch = XCallback;                        // ADDED: replaces XHandler
using XRouter   = XDispatch;                        // ADDED: chain depth 3
