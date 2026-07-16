// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>

namespace mylib {

    // Class with nested enums at all three access levels
    class Device {
    public:
        enum class PowerLevel {
            Low,
            Medium,
            High
        };

    protected:
        enum class State {
            Idle,
            Running,
            Stopped
        };

    private:
        enum class ErrorCode {
            None,
            Timeout,
            Overflow
        };
    };

    // Class with nested typedefs at all three access levels
    class Processor {
    public:
        typedef int    TaskId;
        typedef double ClockFreq;

    protected:
        typedef unsigned int CoreMask;

    private:
        typedef long long CycleCount;
    };

    // Class mixing nested enums and typedefs
    class Controller {
    public:
        typedef int HandleId;

        enum class Mode {
            Manual,
            Auto
        };

    protected:
        typedef float SampleRate;

        enum class Status {
            OK,
            Error
        };

    private:
        typedef unsigned char RegValue;

        enum class Priority {
            Low,
            High
        };
    };

    // Struct with nested enums (struct members are public by default)
    struct Packet {
    public:
        enum class Type {
            Data,
            Control,
            Ack
        };

    protected:
        enum class Encoding {
            Raw,
            Base64
        };

    private:
        enum class Compression {
            None,
            LZ4
        };
    };


    // Class with methods at all three access levels
    class Service {
    public:
        void start();
        int getStatus();

    protected:
        void reset();

    private:
        void shutdown();
    };

}

namespace utils {

    // Class with only enums — all change access
    class Scheduler {
    public:
        enum class Policy {
            FIFO,
            RoundRobin
        };

    protected:
        enum class Clock {
            Monotonic,
            Realtime
        };

    private:
        enum class QueueKind {
            Bounded,
            Unbounded
        };
    };

    // Class with only typedefs — all change access
    class Allocator {
    public:
        typedef void*  RawPtr;
        typedef size_t BlockSize;

    protected:
        typedef unsigned int AlignVal;

    private:
        typedef long long Offset;
    };

}