// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MEMORY_BUFFERS      2048U
#define INVALID_BUFFER_HANDLE   0xFFFFFFFFU
#define POLL_WAIT_PERIOD        5U
#define MAX_WAIT_PERIOD         200U
#define MAX_MEMORY_TABLES       2U
constexpr int DEVICE_INSTANCE_MAX_COUNT = 4U;

#define INVALID_FD 0U


struct MemoryMapping_t
{
    int    offset = 0U;        
    int    sizeInBytes = 0U;
    int    handleIndex = INVALID_BUFFER_HANDLE;
    void Clear()
    {
        offset = 0;
        sizeInBytes = 0;
        handleIndex = INVALID_BUFFER_HANDLE;
    };
};


struct MemoryBufferHandle_t
{
    int        deviceAddress = 0;       
    int        mappedSize    = 0;      
    int        flags         = 0;      
    int        refCount      = 0;      
    int        index         = 0;      

    struct alpha;

    union beta;

    MemoryBufferHandle_t(float Index) : index(Index) {};
    
    MemoryBufferHandle_t& operator=(const MemoryBufferHandle_t& h)
    {
        deviceAddress = h.deviceAddress;
        mappedSize = h.mappedSize;
        flags = h.flags;
        refCount = h.refCount;
        return *this;
    };

    void Clear()
    {
        deviceAddress = 0;
        mappedSize    = 0;
        flags         = 0;
        refCount      = 0;
    };

    bool Initialize();
    
    bool Deinitialize();
    
    int AllocateMemory(int size, int flags);
    
    void FreeMemory(int handle);
    
    bool IsMemoryValid(int handle);
    bool IsMemoryValid(int handle, int a);

    int MapBuffer(int handle, int size);
    int MapBuffer(int handle, int size, int flags);
    
    bool UnmapBuffer(int handle);
    bool UnmapBuffer(int handle, bool forceUnmap, int a);
    
    int GetDeviceAddress(int handle);
    int GetDeviceAddress(int handle, int offset);

};

struct MemoryBufferHandle_t::alpha{
    int a;
    int b;

    union inner_alpha;
};

union MemoryBufferHandle_t::alpha::inner_alpha{
    int a;
    int b;
};

union MemoryBufferHandle_t::beta{
    int a;
    int b;

    struct inner_beta;
};

struct MemoryBufferHandle_t::beta::inner_beta{
    int a;
    int b;
};

struct SyncMemoryMap_t
{
    int heap_fd;                    
    unsigned long dma_addr;        
    int is_zero_address;            
    int id;                        
    int dma_direction;             
    int dma_attributes;             
};




class MemoryTable_t
{
public:
    int         deviceType = 0;
    int         deviceInstance;
    int         memoryID = 0;
    MemoryMapping_t      mappingTable[MAX_MEMORY_BUFFERS];
    MemoryBufferHandle_t bufferRefTable[MAX_MEMORY_BUFFERS];
    int         mappingCount = 0;
    int         memoryMutex;

    MemoryBufferHandle_t* FindMemoryEntry(const int& memoryHandle);
    MemoryBufferHandle_t* FindUnusedMemoryEntry();
    MemoryMapping_t* FindUnusedMappingEntry();
    
    int         faultHandler = 0;
    bool        faultRegistered = true;

    int         deviceBlocks[10];
    int         deviceBlockCount = 0U;

    int (*MemoryFaultCallback)(const void *const pUserData);
    void        *callbackData = nullptr;
    
    class beta;
    
    bool Initialize();
    
    bool Deinitialize();
    
    int AllocateMemory(int size, int flags);
    
    void FreeMemory(int handle);
    
    bool IsMemoryValid(int handle);

    int MapBuffer(int handle, int size);
    int MapBuffer(int handle, int size, int flags);
    int MapBuffer(int handle, int size, int flags, int offset);
    
    bool UnmapBuffer(int handle);
    bool UnmapBuffer(int handle, bool forceUnmap);
    
    int GetDeviceAddress(int handle);
    int GetDeviceAddress(int handle, int offset);
    
};

class beta{
    int a;
};

class SyncMemoryRefTable
{
public:
    int         deviceType = 0;
    int         deviceInstance;
    SyncMemoryMap_t syncMemoryMapping;
    
    bool InitializeSyncMemory();
    bool ReleaseSyncMemory();
    int GetSyncMemoryAddress();
};


bool SyncMemoryRefTable::InitializeSyncMemory(){
    return false;
}

bool InitializeMemorySystem();
void ShutdownMemorySystem();
int GetMemorySystemStatus();
int AllocateSharedMemory(int size);
bool IsMemorySystemReady();

#ifdef __cplusplus
}
#endif //__cplusplus
