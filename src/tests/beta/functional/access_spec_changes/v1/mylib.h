// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>
#include <vector>

namespace mylib {

    struct Serializable {
        int serial_version;
    };

    struct Loggable {
        int log_id;
    };

    struct Auditable {
        long long audit_timestamp;
    };

    struct Cacheable {
        bool cache_valid;
    };

    struct Validatable {
        bool is_valid;
    };

    struct Identifiable {
        int uid;
    };

    class Config : public Serializable, protected Loggable, private Auditable {
    public:
        std::string name;
        int         version;
        bool        debug_mode;

    protected:
        std::string internal_tag;
        int         retry_count;

    private:
        std::string secret_key;
        double      timeout_seconds;
    };

    class Logger : public Loggable, public Auditable, protected Validatable {
    public:
        std::string log_prefix;
        int         log_level;

    protected:
        std::string log_file_path;
        bool        append_mode;

    private:
        std::vector<std::string> log_buffer;
        int                      max_buffer_size;
    };

    namespace network {

        struct Connectable {
            bool connected;
        };

        struct Retryable {
            int retry_limit;
        };

        struct Compressible {
            int compression_level;
        };

        class Socket : public Connectable, public Retryable, protected mylib::Loggable {
        public:
            std::string host;
            int         port;

        protected:
            int  socket_fd;
            bool blocking;

        private:
            std::string  protocol;
            unsigned int timeout_ms;
        };

        class HttpClient : public Connectable, protected Retryable, protected mylib::Serializable, private mylib::Auditable {
        public:
            std::string base_url;
            int         max_redirects;

        protected:
            std::string auth_token;
            bool        verify_ssl;

        private:
            std::vector<std::string> headers;
            int                      connection_pool_size;
        };

    }

    namespace storage {

        struct Persistable {
            std::string storage_path;
        };

        struct Versionable {
            int schema_version;
        };

        struct Expirable {
            long long expiry_time;
        };

        class Record : public Persistable, public mylib::Serializable, protected mylib::Auditable, private mylib::Identifiable {
        public:
            int         id;
            std::string table_name;

        protected:
            std::string raw_data;
            bool        is_dirty;

        private:
            std::string checksum;
            long long   created_at;
        };

        class Cache : public mylib::Cacheable, protected Versionable, private Expirable {
        public:
            int  capacity;
            bool enabled;

        protected:
            int  hit_count;
            int  miss_count;

        private:
            std::vector<std::string> keys;
            double                   eviction_ratio;
        };

    }

}

namespace utils {

    struct Measurable {
        double measurement;
    };

    struct Resettable {
        bool reset_on_start;
    };

    struct Formattable {
        std::string format_string;
    };

    struct Encodable {
        int encoding_type;
    };

    class Timer : public Measurable, public Resettable, protected mylib::Loggable {
    public:
        std::string label;
        bool        auto_reset;

    protected:
        long long start_time;
        long long elapsed_ms;

    private:
        int  tick_interval;
        bool running;
    };

    class StringHelper : public Formattable, protected Encodable, private mylib::Validatable {
    public:
        char        delimiter;
        bool        trim_whitespace;

    protected:
        std::string locale;
        int         max_length;

    private:
        std::vector<std::string> token_cache;
        bool                     case_sensitive;
    };

    namespace math {

        struct Transformable {
            float scale;
        };

        struct Interpolatable {
            float t;
        };

        struct Projectable {
            float fov;
        };

        class Vector3 : public Transformable, public Interpolatable, protected mylib::Serializable {
        public:
            float x;
            float y;
            float z;

        protected:
            float magnitude;
            bool  normalized;

        private:
            float raw_data[3];
            int   precision;
        };

        class Matrix4 : public Transformable, protected Projectable, private Interpolatable {
        public:
            int  rows;
            int  cols;

        protected:
            float determinant;
            bool  is_identity;

        private:
            float data[4][4];
            bool  transposed;
        };

    }

}
