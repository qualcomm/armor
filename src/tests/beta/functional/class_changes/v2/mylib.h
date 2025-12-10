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

    MemoryBufferHandle_t(float Index, int a) : index(Index) {};
    
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
    void FreeMemory(int handle, int flags);
    
    bool IsMemoryValid(int handle);

    int MapBuffer(int handle, int size);
    int MapBuffer(int handle, int size, int flags);
    
    bool UnmapBuffer(int handle, int a);
    bool UnmapBuffer(int handle, bool forceUnmap, int a);
    
    int GetDeviceAddress(int handle);
    int GetDeviceAddress(int handle, int offset);

};

int MemoryBufferHandle_t::MapBuffer(int handle, int size, int flags){
    return 0;
}

struct MemoryBufferHandle_t::alpha{
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
    
    bool Initialize();
    
    bool Deinitialize();
    
    int AllocateMemory(int size, int flags);
    
    void FreeMemory(int handle);
    void FreeMemory(int handle, int flags);
    
    bool IsMemoryValid(int handle);

    int MapBuffer(int handle, int size);
    int MapBuffer(int handle, int size, int flags);
    
    bool UnmapBuffer(int handle);
    bool UnmapBuffer(int handle, bool forceUnmap, int a);
    
    int GetDeviceAddress(int handle);
    int GetDeviceAddress(int handle, int offset);
    
};

class SyncMemoryRefTable
{
public:
    int         deviceType = 0;
    int         deviceInstance;
    SyncMemoryMap_t syncMemoryMapping;
    
    bool ReleaseSyncMemory();
    int GetSyncMemoryAddress();
};

bool InitializeMemorySystem();
void ShutdownMemorySystem();
int GetMemorySystemStatus();
bool IsMemorySystemReady();
int AllocateSharedMemory(int size);
void FreeSharedMemory(int handle);

#ifdef __cplusplus
}
#endif //__cplusplus
