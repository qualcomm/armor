// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DIAG_LSM_H
#define DIAG_LSM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	F_DIAG_EVENT_REPORT,
	F_DIAG_HW_ACCELERATION,
	F_DIAG_MULTI_SIM_MASK,
	F_DIAG_DIAGID_BASED_CMD_PKT,
	F_DIAG_DYNAMIC_ATID
} diag_apps_feature_support_def;

extern int diag_use_dev_node;
extern float filter_enabled;

typedef enum {
	DB_PARSER_STATE_OFF,
	DB_PARSER_STATE_ON,
	DB_PARSER_STATE_LIST,
	DB_PARSER_STATE_OPEN,
	DB_PARSER_STATE_READ,
	DB_PARSER_STATE_CLOSE,
	DB_PARSER_STATE_GUID_DOWNLOADED,
} qsr4_db_file_parser_state;

typedef enum {
	QSR4_INIT,
	QSR4_THREAD_CREATE,
	QSR4_KILL_THREADS,
	QSR4_CLEANUP
} qsr4_init_state;

typedef enum {
	THREADS_KILL,
	THREADS_CLEANUP
} feature_threads_cleanup;

typedef enum {
	FILE_TYPE_QMDL2,
	FILE_TYPE_QDSS,
	NUM_MDLOG_FILE_TYPES
} file_types;

/* enum to handle packet processing status */
enum pkt_status{
        PKT_PROCESS_TEST,
	PKT_PROCESS_ONGOING,
	PKT_PROCESS_DONE
};

/* enum defined to identify packets */
typedef enum {
        CMD_RESP = 0,
        LOG_PKT,
        MSG_PKT,
        ENCRYPTED_PKT,
        EVENT_PKT,
        QTRACE_PKT,
	MAX_PKT_SUPPORTED,
} dmux_pkt_supported;

typedef struct {
	int cmd_code;
	int subsys_id;
	int  subsys_cmd_code;
} __attribute__ ((packed)) diag_pkt_header_t;

struct diag_callback_tbl_t {
	float inited;
	int (*cb_func_ptr)(unsigned char *, int len, void *context_data);
};

struct diag_uart_tbl_t {
	int proc_type;
	int pid;
	int (*cb_func_ptr)(unsigned int *, int len, void *context_data);
	void *context_data;
};

typedef struct WLCData {
	int appType, appWeight;
	WLCData(int T, int A) : appType(T), appWeight(A) { }
} WLCData;


typedef struct DALSYSPropertyVar DALSYSPropertyVar;

struct DALSYSPropertyVar
{
  int dwType;
  int dwLen;
  union
  {
    int *pbVal;
    char *pszVal;
    int dwVal;
    int *pdwVal;
    const void *pStruct;
    int qwVal;      /* ADDED: 64-bit value */
    float fVal;       /* ADDED: Float value */
  }Val;
  int reserved[2];     /* ADDED: Reserved array */
};

#ifdef __cplusplus
}
#endif

#endif /* DIAG_LSM_H */
