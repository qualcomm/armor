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

    class Config : private Serializable, public Loggable, public Validatable {
    public:
        std::string internal_tag;
        int         retry_count;
        double      timeout_seconds;

    protected:
        int         version;
        std::string secret_key;

    private:
        std::string name;
        bool        debug_mode;
    };

    class Logger : protected Loggable, private Auditable, public Identifiable, public Serializable {
    public:
        std::string              log_file_path;
        std::vector<std::string> log_buffer;

    protected:
        std::string log_prefix;
        int         max_buffer_size;

    private:
        int  log_level;
        bool append_mode;
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

        class Socket : protected Connectable, public mylib::Loggable, private Compressible {
        public:
            int         socket_fd;
            std::string protocol;

        protected:
            int          port;
            unsigned int timeout_ms;

        private:
            std::string host;
            bool        blocking;
        };

        class HttpClient : private Connectable, public Retryable, protected mylib::Auditable, public Compressible {
        public:
            std::string              auth_token;
            std::vector<std::string> headers;

        protected:
            int max_redirects;
            int connection_pool_size;

        private:
            std::string base_url;
            bool        verify_ssl;
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

        class Record : private Persistable, public mylib::Serializable, public mylib::Auditable, protected Versionable {
        public:
            std::string raw_data;
            std::string checksum;

        protected:
            int       id;
            long long created_at;

        private:
            std::string table_name;
            bool        is_dirty;
        };

        class Cache : private mylib::Cacheable, public Versionable, public mylib::Validatable, protected Persistable {
        public:
            int                      hit_count;
            std::vector<std::string> keys;

        protected:
            bool   enabled;
            double eviction_ratio;

        private:
            int  capacity;
            int  miss_count;
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

    class Timer : protected Measurable, private Resettable, public mylib::Auditable, public Formattable {
    public:
        long long start_time;
        int       tick_interval;

    protected:
        bool auto_reset;
        bool running;

    private:
        std::string label;
        long long   elapsed_ms;
    };

    class StringHelper : private Formattable, public Encodable, public Resettable, protected mylib::Loggable {
    public:
        std::string              locale;
        std::vector<std::string> token_cache;

    protected:
        char delimiter;
        bool case_sensitive;

    private:
        bool trim_whitespace;
        int  max_length;
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

        class Vector3 : private Transformable, public mylib::Serializable, public Projectable, protected Interpolatable {
        public:
            float z;
            float magnitude;
            float raw_data[3];

        protected:
            float y;
            int   precision;

        private:
            float x;
            bool  normalized;
        };

        class Matrix4 : public Transformable, private Projectable, public Interpolatable, protected mylib::Serializable {
        public:
            float determinant;
            float data[4][4];

        protected:
            int  cols;
            bool transposed;

        private:
            int  rows;
            bool is_identity;
        };

    }

}
