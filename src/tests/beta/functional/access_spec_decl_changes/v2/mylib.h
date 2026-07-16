// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>

namespace mylib {

    // Access specifiers shuffled: public->protected, protected->private, private->public
    class Device {
    protected:
        enum class PowerLevel {
            Low,
            Medium,
            High
        };

    private:
        enum class State {
            Idle,
            Running,
            Stopped
        };

    public:
        enum class ErrorCode {
            None,
            Timeout,
            Overflow
        };
    };

    // Typedef access shuffled: public->private, protected->public, private->protected
    class Processor {
    private:
        typedef int    TaskId;
        typedef double ClockFreq;

    public:
        typedef unsigned int CoreMask;

    protected:
        typedef long long CycleCount;
    };

    // Mixed: typedef public->protected, enum public->private,
    //        typedef protected->public, enum protected->public,
    //        typedef private->public, enum private->protected
    class Controller {
    protected:
        typedef int HandleId;

    private:
        enum class Mode {
            Manual,
            Auto
        };

    public:
        typedef float SampleRate;

    public:
        enum class Status {
            OK,
            Error
        };

    public:
        typedef unsigned char RegValue;

    protected:
        enum class Priority {
            Low,
            High
        };
    };

    // Struct nested enums shuffled: public->private, protected->public, private->protected
    struct Packet {
    private:
        enum class Type {
            Data,
            Control,
            Ack
        };

    public:
        enum class Encoding {
            Raw,
            Base64
        };

    protected:
        enum class Compression {
            None,
            LZ4
        };
    };


    // Methods shuffled: public->private, protected->public, private->protected
    class Service {
    private:
        void start();
        int getStatus();

    public:
        void reset();

    protected:
        void shutdown();
    };

}

namespace utils {

    // Scheduler enums shuffled: public->private, protected->public, private->protected
    class Scheduler {
    private:
        enum class Policy {
            FIFO,
            RoundRobin
        };

    public:
        enum class Clock {
            Monotonic,
            Realtime
        };

    protected:
        enum class QueueKind {
            Bounded,
            Unbounded
        };
    };

    // Allocator typedefs shuffled: public->protected, protected->private, private->public
    class Allocator {
    protected:
        typedef void*  RawPtr;
        typedef size_t BlockSize;

    private:
        typedef unsigned int AlignVal;

    public:
        typedef long long Offset;
    };

}