/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_CORE_UTILS__H_
#include "RaidCore_Common.h"
#include "RaidCore_MemManager.h"

//--------------------------------------------------------------------------------------------------------------------------------------------------
// Cyclic buffer
namespace game_buffer_func
{
	struct rc_cyclic_buffer
	{
		uint8 * buffer;
		uint32 length;
        uint32 readPosition;
        uint32 writePosition;
    };
	//--------------------------------------------------------------------------------------------------------------------------------------------------
	uint32 ReadBuffer(rc_cyclic_buffer* buffer, uint32 length, uint8* dst);
    uint32 WriteBuffer(rc_cyclic_buffer* buffer, uint32 length, uint8* src, uint32 srcLength);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
// Template item list
#define RC_LIST_ITEMCOUNT 16
namespace rc_list {
    template <class T> struct block {
        uint32 count;
        block<T> * next;
        T items[RC_LIST_ITEMCOUNT];
    };
    template <class T> struct iterator {
        bool32 continueOnAdvance;
        volatile uint32 at;
        block<T> * current;

        T* operator ++(int) {
            _WriteBarrier();

            if (current && at == current->count) {
                at = 0;
                current = current->next;
            }
            if (!current) {
                return NULL;
            }
            T* result = (current->items + at++);
            return(result);
        }
        void next() {
            _WriteBarrier();

            if (continueOnAdvance && current) {
                if (at == current->count) {
                    at = 0;
                    current = current->next;
                }
                else {
                    ++at;
                }
            } else {
                continueOnAdvance = true32;
            }
        }
        T* selected() {
            _WriteBarrier();

            while (NULL != current) {
                if (at < current->count) {
                    return(current->items + at);
                }
                at = 0;
                current = current->next;
            }
            return NULL;
        }
        bool32 has() {
            _WriteBarrier();
            
            if (NULL != current) {
                if (at < current->count) {
                    return true32;
                }
                if (current->next) {
                    return true32;
                }
            }
            return false32;
        }
    };
    template <class T> struct list {
        uint32 count;
        block<T> *firstBlock;
        static block<T> *freeBlock;
        typedef T __type;
    };
    template <class T> static list<T> initList() {
        list<T> reply = { 0 };
        return(reply);
    }
    template <class T> iterator<T> getIterator(list<T>& ref) {
        iterator<T> i = { ref.count };
        i.current = ref.firstBlock;
        i.continueOnAdvance = true32;
        return(i);
    }
    template <class T> bool32 hasValues(list<T>& ref) {
        return(ref.count != 0);
    }
    template <class T> uint32 count(list<T>& ref) {
        return(ref.count);
    }
    template <class T> void push(list<T>& ref, const T& item, game_memory::arena_p arena) {
        if (!ref.firstBlock || ref.firstBlock->count == RC_LIST_ITEMCOUNT) {
            block<T> * oldFirst = NULL;
            if (ref.freeBlock) {
                oldFirst = ref.freeBlock;
                ref.freeBlock = ref.freeBlock->next;
            } else {
                if (arena) {
                    oldFirst = (block<T>*)PushSizeP(arena, sizeof(block<T>));
                } else {
                    AssertInvalidCodePath;
                }
            }
            oldFirst->next = ref.firstBlock;
            ref.firstBlock = oldFirst;
        }
        ++ref.count;
        ref.firstBlock->items[ref.firstBlock->count++] = item;
    }
    template <class T> void remove(list<T>& ref, iterator<T>& i) {
        if (i.selected()) {
            block<T> * Block = i.current;
            Assert(ref.firstBlock->count > 0);
            //
            --ref.count;
            --ref.firstBlock->count;
            if (!(Block == ref.firstBlock && i.at == ref.firstBlock->count)) {
                Block->items[i.at] = ref.firstBlock->items[ref.firstBlock->count];
            }
            ref.firstBlock->items[ref.firstBlock->count] = { 0 };
            i.continueOnAdvance = (Block != ref.firstBlock || i.at >= ref.firstBlock->count);

            if (ref.firstBlock->count == 0) {
                Block = ref.firstBlock->next;
                ref.firstBlock->next = ref.freeBlock;
                ref.freeBlock = ref.firstBlock;
                ref.firstBlock = Block;

                if (i.current == ref.freeBlock) {
                    i.current = ref.firstBlock;
                    i.at = 0;
                    i.continueOnAdvance = false32;
                }
            }
        }
    }
    template <class T> void remove(list<T>& ref, T item) {
        block<T> * Block = ref.firstBlock;
        while (Block) {
            for (uint32 i = 0; i < Block->count; ++i) {
                if (item == Block->items[i]) {
                    Assert(ref.firstBlock->count > 0);
                    Block->items[i] = ref.firstBlock->items[--firstBlock->count];
                    --ref.count;

                    if (ref.firstBlock->count == 0) {
                        Block = ref.firstBlock->next;
                        ref.firstBlock->next = ref.freeBlock;
                        ref.freeBlock = ref.firstBlock;
                        ref.firstBlock = Block;
                    }
                    return;
                }
            }
            Block = Block->next;
        }
    }
    template <class T> T pop(list<T>& ref) {
        T reply = {};
        if (ref.firstBlock) {
            reply = ref.firstBlock->items[0];
            block<T, n> * Block = ref.firstBlock;
            Assert(ref.firstBlock->count > 0);
            Block->items[0] = ref.firstBlock->items[--firstBlock->count];
            --ref.count;

            if (ref.firstBlock->count == 0) {
                if (ref.firstBlock->count == 0) {
                    Block = ref.firstBlock->next;
                    ref.firstBlock->next = ref.freeBlock;
                    ref.freeBlock = ref.firstBlock;
                    ref.firstBlock = Block;
                }
            }
        }
        return(reply);
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
// TIMING!!!
namespace game_timer
{
	struct RaidTick;
	void Init(RaidTick*);

	int64 Diff(RaidTick*) ; // 1 miliseconds passed
	void Mark(RaidTick*) ; // 1 miliseconds passed

    real32 getElapsedSeconds(RaidTick*); // 1 miliseconds passed
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
// BINARY SORTING & SEARCH
/*
 length is the count of elements in A, not the last index!!!!
 meaning A[length] points to the area out of bounds!!!
 */
template<class T> bool32 binarySearch(T* A, T key, const uint32 length, T** pOutIndex) {
    register uint32 imid;
    register uint32 imin = 0;
    register uint32 imax = length;
    // continually narrow search until just one element remains
    while (imin < imax) {
        imid = imin + ((imax - imin) >> 1);

        // reduce the search
        if (A[imid] < key)
            imin = imid + 1;
        else
            imax = imid;
    }
    // deferred test for equality
    if (imin == length) {
        return false32;
    }
    (*pOutIndex) = (A + imin);
    return true32;
}
//
template<class T> void SwapMemBlock(T* L, T* R)
{
	T _tmp ;
	memcpy(&_tmp, L, sizeof(T));
	memcpy(L, R, sizeof(T));
	memcpy(R, &_tmp, sizeof(T));
}
//
namespace sort_functions {
    namespace {
        template<class T> int32 pivot(T* A, int32 beg, int32 end) {
            T pivotElement = A[end];
            int32 pi = beg - 1;

            for (int32 i = beg; i <= end - 1; ++i) {
                /* If you want to sort the list in the other order, change "<=" to ">" */
                if (A[i] <= pivotElement) {
                    ++pi;
                    if (pi != i) {
                        SwapMemBlock<T>(A + i, A + pi);
                    }
                }
            }

            SwapMemBlock<T>(A + (pi + 1), A + end);
            return (pi + 1);
        }
    }
    //------------------------------------------------------------------
    //	quick sort
    template<class T> void quickSort(T* A, int32 beg, int32 end) {
        int32 pivotElement;
        if (beg < end) {
            pivotElement = pivot<T>(A, beg, end);
            quickSort<T>(A, beg, pivotElement - 1);
            quickSort<T>(A, pivotElement + 1, end);
        }
    }

    namespace {
        //
        //
        //  Left half is A[iBegin:iMiddle-1].
        // Right half is A[iMiddle:iEnd-1   ].
        template<class T> void arrayMerge(T* A, uint32 iBegin, uint32 iMiddle, uint32 iEnd, T* B) {
            uint32 i = iBegin, j = iMiddle;
            // While there are elements in the left or right runs...
            for (uint32 k = iBegin; k < iEnd; ++k) {
                T& Bk = *(B + k);

                // If left run head exists and is <= existing right run head.
                //if (i < iMiddle && (j >= iEnd || Ai <= Aj)) {
                if (i < iMiddle && j < iEnd) {
                    T& Ai = *(A + i);
                    T& Aj = *(A + j);
                    if (Ai <= Aj) {
                        Bk = Ai;
                        ++i;
                    } else {
                        Bk = Aj;
                        ++j;
                    }
                } else if (i < iMiddle) {
                    Bk = *(A + i);
                    ++i;
                } else if (j < iEnd) {
                    Bk = *(A + j);
                    ++j;
                }
            }
            for (uint32 k = iBegin; k < iEnd; ++k) {
                *(A + k) = *(B + k);
            }
        }
        //
        // iBegin is inclusive; iEnd is exclusive (A[iEnd] is not in the set).
        template<class T> void splitMerge(T* A, uint32 iBegin, uint32 iEnd, T* B) {
            if (iEnd - iBegin == 1) {                   // if run size == 1
                return;                                 //   consider it sorted
            } else if (iEnd - iBegin == 2) {
                T& Ai = *(A + iBegin);
                T& Aj = *(A + iBegin + 1);
                if (Ai > Aj) {
                    SwapMemBlock<T>(A + iBegin, A + iBegin + 1);
                }
                return;
            }
            // recursively split runs into two halves until run size == 1,
            // then merge them and return back up the call chain
            uint32 iMiddle = (iEnd + iBegin) / 2;              // iMiddle = mid point
            splitMerge<T>(A, iBegin, iMiddle, B);  // split / merge left  half
            splitMerge<T>(A, iMiddle, iEnd, B);  // split / merge right half
            arrayMerge<T>(A, iBegin, iMiddle, iEnd, B);  // merge the two half runs
        }
    }
    //
    // Array A[] has the items to sort; array B[] is a work array.
    template<class T> void mergeSort(T* inArray, game_memory::arena_p tempMemory, uint32 arrayLength) {
        TemporaryMemory _t = BeginTemporaryMemory(tempMemory);
        T* temporary = PushArray(_t.arena, arrayLength, T);
        splitMerge<T>(inArray, 0, arrayLength, temporary);
        EndTemporaryMemory(_t);
    }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
// Template HashMap;
namespace hash_map {
	template<class Type> struct block
	{
		block *
			_next;
		Type
			_data;
		intptr
			_key;
	};
    typedef intptr(*hashKeyFunction)(intptr _key);
    rc_macro_force intptr defaultKeyEncode(intptr _key) {
        return _key;
    }
    template<class Type> rc_macro_force bool32 defaultHashCompareLess(const block<Type>& left, const block<Type>& right) {
        return (left._key < right._key);
    }
	template<class Type> struct map
	{
		game_memory::arena_p
			_hashMemoryAlloc;
		uint32
			_blockCount; // how many blocks were allocated
		block<Type>**
			_hashBlocks;
		//
		static block<Type>*
			_firstFree;	// firstBlock freeBlock hash element
        //
        uint32
            _elementCount;
        //
        hashKeyFunction encodeKey;
        typedef bool32(*hashCompareLessFunction)(const block<Type>& left, const block<Type>& right);
        hashCompareLessFunction elementCompare;
        //
        intptr _firstSetBlock;
	};
    template<class Type> rc_macro_force intptr hashKey(map<Type> * map, intptr _key) {
        Assert(map->_blockCount);
        intptr true_key = game_math::absolute(map->encodeKey(_key)) % map->_blockCount;
        return (true_key);
    }
    template<class Type> void initialize(map<Type>* map, game_memory::arena_p mem, uint32 count)
	{
		map->_hashMemoryAlloc = mem;
		map->_blockCount = count;
		map->_firstFree = NULL;
        map->_elementCount = 0;
        map->encodeKey = defaultKeyEncode;
        map->elementCompare = defaultHashCompareLess<Type>;
        map->_firstSetBlock = count;
		//
		// Point to firstBlock block
		map->_hashBlocks = (block<Type>**)game_memory::alloc(mem, sizeof(block<Type>*) * count).m_pMemory;
        if (sizeof(Type) & 0x01) {
            setBytes8(map->_hashBlocks, 0, sizeof(Type) * count);
        } else if (sizeof(Type) & 0x03) {
            setBytes16(map->_hashBlocks, 0, sizeof(Type) * count);
        } else if (sizeof(Type) & 0x07) {
            setBytes32(map->_hashBlocks, 0, sizeof(Type) * count);
        }
    }
    template<class Type> Type* add(map<Type> * map, Type& object, intptr _key) {
        intptr true_key = hashKey(map, _key);
        if (map->_firstSetBlock > true_key) {
            map->_firstSetBlock = true_key;
        }
        //
        block<Type>* e;
        if (NULL == map->_firstFree) {
            e = (block<Type>*)game_memory::alloc(map->_hashMemoryAlloc, sizeof(block<Type>)).m_pMemory;
        } else {
            e = map->_firstFree;
            map->_firstFree = map->_firstFree->_next;
        }
        // set hash block data
        e->_key = _key;
        e->_data = object;
        // insert new hash block
        if (NULL == map->_hashBlocks[true_key]) {
            map->_hashBlocks[true_key] = e;
            e->_next = NULL;
        } else {
            block<Type>* eptr = map->_hashBlocks[true_key];
            e->_next = eptr;
            map->_hashBlocks[true_key] = e;
        }
        //
        ++map->_elementCount;
        return &(e->_data);
    }
    template<class Type> Type* addDesc(map<Type> * map, Type& object, intptr _key) {
        intptr true_key = hashKey(map, _key);
        if (map->_firstSetBlock > true_key) {
            map->_firstSetBlock = true_key;
        }
        //
        block<Type>* e;
        if (NULL == map->_firstFree) {
            e = (block<Type>*)game_memory::alloc(map->_hashMemoryAlloc, sizeof(block<Type>)).m_pMemory;
        } else {
            e = map->_firstFree;
            map->_firstFree = map->_firstFree->_next;
        }
        // set hash block data
        e->_key = _key;
        e->_data = object;
        // insert new hash block
        if (NULL == map->_hashBlocks[true_key]) {
            map->_hashBlocks[true_key] = e;
            e->_next = NULL;
        } else {
            block<Type>* head = NULL;
            block<Type>* eptr = map->_hashBlocks[true_key];
            while (NULL != eptr && map->elementCompare(*e, *eptr)) {
                head = eptr;
                eptr = eptr->_next;
            }
            e->_next = eptr;
            if (NULL == head) {
                map->_hashBlocks[true_key] = e;
            } else {
                head->_next = e;
            }
        }
        //
        ++map->_elementCount;
        return &(e->_data);
    }
    template<class Type> Type* addAsc(map<Type> * map, Type& object, intptr _key) {
        intptr true_key = hashKey(map, _key);
        if (map->_firstSetBlock > true_key) {
            map->_firstSetBlock = true_key;
        }
        //
        block<Type>* e;
        if (NULL == map->_firstFree) {
            e = (block<Type>*)game_memory::alloc(map->_hashMemoryAlloc, sizeof(block<Type>)).m_pMemory;
        } else {
            e = map->_firstFree;
            map->_firstFree = map->_firstFree->_next;
        }
        // set hash block data
        e->_key = _key;
        e->_data = object;
        // insert new hash block
        if (NULL == map->_hashBlocks[true_key]) {
            map->_hashBlocks[true_key] = e;
            e->_next = NULL;
        } else {
            block<Type>* head = NULL;
            block<Type>* eptr = map->_hashBlocks[true_key];
            while (NULL != eptr && map->elementCompare(*eptr, *e)) {
                head = eptr;
                eptr = eptr->_next;
            }
            e->_next = eptr;
            if (NULL == head) {
                map->_hashBlocks[true_key] = e;
            } else {
                head->_next = e;
            }
        }
        //
        ++map->_elementCount;
        return &(e->_data);
    }
    template<class Type> block<Type>* getList(map<Type> * map, intptr _key)
	{
        intptr true_key = hashKey(map, _key);
        return map->_hashBlocks[true_key];
	}
    template<class Type> bool32 isSet(map<Type>* map, intptr _key)
	{
        intptr true_key = hashKey(map, _key);
        block<Type>* block = map->_hashBlocks[true_key];
        while (block) {
            if (block->_key != _key) {
                block = block->_next;
                continue;
            }
            return true32;
        }
        return false32;
	}
    template<class Type> bool32 get(map<Type>* map, intptr _key, Type *& result)
	{
        intptr true_key = hashKey(map, _key);

        block<Type>* block = map->_hashBlocks[true_key];
        while (block) {
            if (block->_key != _key) {
                block = block->_next;
                continue;
            }
            result = &block->_data;
            return true32;
        }
        return false32;
	}
	template<class Type> void remove(map<Type> * map, intptr _key)
	{
        intptr true_key = hashKey(map, _key);
        //
        block<Type>* head = NULL;
        block<Type>* eptr = map->_hashBlocks[true_key];
        while (NULL != eptr && eptr->_key != _key) {
            head = eptr;
            eptr = eptr->_next;
        }
        if (NULL != eptr) {
            if (NULL == head) {
                map->_hashBlocks[true_key] = eptr->_next;
            } else {
                head->_next = eptr->_next;
            }
            //
            --map->_elementCount;
            //
            eptr->_next = map->_firstFree;
			map->_firstFree = eptr;
        }
	}
    template<class Type> bool32 pop(map<Type> * map, Type* object) {
        bool32 result = false32;
        while (map->_firstSetBlock < map->_blockCount) {
            //
            block<Type>* eptr = map->_hashBlocks[map->_firstSetBlock];
            if (NULL != eptr) {
                map->_hashBlocks[map->_firstSetBlock] = eptr->_next;
                //
                --map->_elementCount;
                //
                eptr->_next = map->_firstFree;
                map->_firstFree = eptr;

                *object = eptr->_data;
                result = true32;
                break;
            }
            ++map->_firstSetBlock;
        }
        return (result);
    }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
// Template Queue;
namespace rc_queue {
    template<class Type> struct block {
        block *
            _prev,
            _next;
        Type
            _data;
    };
    template<class Type> struct queue {
        game_memory::arena_p
            _queueMemoryAlloc;
        uint32
            _blockCount; // how many blocks were allocated
        block<Type>*
            _queueStart,
            _queueEnd;
        //
        static block<Type>*
            _firstFree;	// firstBlock freeBlock queue element
    };
    template<class Type> void initializeQueue(queue<Type> * map, game_memory::arena_p mem) {
        map->_queueMemoryAlloc = mem;
        map->_blockCount = 0;
        map->_queueEnd = map->_queueStart = NULL;
        map->_firstFree = NULL;
    }
    template<class Type> void insert(queue<Type> * map, Type& object) {
        if (map->_firstFree == NULL) {
            map->_firstFree = PushStruct(map->_queueMemoryAlloc, block<Type>);
            *(map->_firstFree) = {};
        }
        //
        block<Type>* e = map->_firstFree;
        map->_firstFree = map->_firstFree->_next;
        //
        if (map->_queueEnd == NULL) {
            map->_queueStart = map->_queueEnd = e;
        }
        else {
            e->_next = map->_queueStart;
            map->_queueStart->_prev = e;
            map->_queueStart = e;
        }
        e->_data = object;
        ++(map->_blockCount);
    }
    template<class Type> void push(queue<Type> * map, Type& object) {
        if (map->_firstFree == NULL) {
            map->_firstFree = PushStruct(map->_queueMemoryAlloc, block<Type>);
            *(map->_firstFree) = {};
        }
        //
        block<Type>* e = map->_firstFree;
        map->_firstFree = map->_firstFree->_next;
        //
        if (map->_queueEnd == NULL) {
            map->_queueStart = map->_queueEnd = e;
        }
        else {
            e->_prev = map->_queueEnd;
            map->_queueEnd->_next = e;
            map->_queueEnd = e;
        }
        e->_data = object;
        ++(map->_blockCount);
    }
    template<class Type> Type& firstBlock(queue<Type> * map) {
        if (map->_blockCount != 0) {
            return (map->_queueStart->_data);
        }
        return{};
    }
    template<class Type> Type& last(queue<Type> * map) {
        if (map->_blockCount != 0) {
            return (map->_queueEnd->_data);
        }
        return{};
    }
    template<class Type> void pop(queue<Type> * map) {
        if (map->_blockCount != 0) {
            block<Type> * e = map->_queueEnd;
            if (NULL != map->_queueEnd->_prev) {
                map->_queueEnd = map->_queueEnd->_prev;
            }
            else {
                map->_queueEnd = map->_queueStart = NULL;
            }
            //
            if (NULL != map->_firstFree) {
                e->_next = map->_firstFree;
            }
            e->_prev = NULL;
            map->_firstFree = e;
            --(map->_blockCount);
        }
    }
    template<class Type> void dequeue(queue<Type> * map) {
        if (map->_blockCount != 0) {
            block<Type> * e = map->_queueStart;
            if (NULL != map->_queueStart->_next) {
                map->_queueStart = map->_queueStart->_next;
            } else {
                map->_queueEnd = map->_queueStart = NULL;
            }
            //
            if (NULL != map->_firstFree) {
                e->_next = map->_firstFree;
            }
            e->_prev = NULL;
            map->_firstFree = e;
            --(map->_blockCount);
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------
// Template Octree;

//--------------------------------------------------------------------------------------------------------------------------------------------------
// Graph
namespace rc_graph {
    struct GraphEdge {
        real32 _weight;
        uint32 _first;
        uint32 _second;
    };
    struct GraphNode {
        GraphNode* _pathFrom;
        real32 _pathWeight = FLT_MAX;
        bool32 _seen = false32;
        rc_list::list<GraphEdge> _edges;
    };
    struct Graph {
        uint32 _count;
        hash_map::map<GraphNode> _nodes;
    };

    inline Graph * newGraph(game_memory::arena_p memory) {
        Graph * graph = PushStruct(memory, Graph);
        hash_map::initialize(&graph->_nodes, memory, 256);
        return graph;
    }
    inline uint32 addNode(Graph* graph, game_memory::arena_p memory) {
        hash_map::add<GraphNode>(&graph->_nodes, GraphNode(), graph->_count & 256);
        return graph->_count++;
    }
    inline void addEdge(Graph* graph, uint32 from, uint32 to, real32 weight, game_memory::arena_p memory) {
        GraphEdge edge = { weight, from, to };//>(&graph->_nodes, GraphNode());
        uint32 fromKey = edge._first & 256;
        uint32 toKey = edge._second & 256;
        hash_map::block<GraphNode>* fromNode = hash_map::getList<GraphNode>(&graph->_nodes, fromKey);
        hash_map::block<GraphNode>* toNode = hash_map::getList<GraphNode>(&graph->_nodes, toKey);
        while ((NULL != fromNode) && (fromNode->_key != fromKey)) {
            fromNode = fromNode->_next;
        }
        while ((NULL != toNode) && (toNode->_key != toKey)) {
            toNode = toNode->_next;
        }
        if (NULL == fromNode || NULL == toNode) {
            return;
        }
        rc_list::push(fromNode->_data._edges, edge, memory);
        // inplace swap of integers
        edge._first += edge._second;
        edge._second = edge._first - edge._second;
        edge._first = edge._first - edge._second;
        rc_list::push(toNode->_data._edges, edge, memory);
    }
    inline GraphNode * followEdge(Graph* graph, GraphEdge* path) {
        uint32 toKey = path->_second & 256;
        hash_map::block<GraphNode>* toNode = hash_map::getList<GraphNode>(&graph->_nodes, toKey);
        while ((NULL != toNode) && (toNode->_key != toKey)) {
            toNode = toNode->_next;
        }
        if (NULL == toNode) {
            return NULL;
        }
        GraphNode * node = &toNode->_data;
        return (node);
    }
}

#define __RC_X_CORE_UTILS__H_
#endif //__RC_X_CORE_UTILS__H_

