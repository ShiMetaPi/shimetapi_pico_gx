#ifndef __DATA_BASE__
#define __DATA_BASE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "sqlite3.h"
#include "xmedia_type.h"
#include "xmedia_svp.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MALLOC_FAILED 3
#define INIT_FAILED 4
#define DOT_FAILED 5
#define SQL_ERROR 6
#define MATCH_FAILED 7
#define PUSH_FAILED 8

#define SQL_COL_NUM 2
#define SQL_ROW_NUM 10

#define MAX_ROWS 30
#define MAX_COLS 512
#define MATCH_THRES 0.003
#define MAX_SIZE 1000

#define MAX_TABLE_NAME_LEN 100

#define MAX_NAME_LEN 100
#define MAX_INDEX_LEN 4

#define MAX_DB_NUM 2

#define NAME_LEN 20
#define MAX_NAME_LEN 100
#define MAX_FEATURE_LEN 10800

#define MAX_MATCH_NUM 10

typedef struct {
    xmedia_double data[MAX_ROWS][MAX_COLS];
    xmedia_s32 row_num;
    xmedia_s32 col_num;
} matrix_data;

typedef struct {
    xmedia_double data[MAX_COLS][MAX_ROWS];
    xmedia_s32 row_num;
    xmedia_s32 col_num;
} t_matrix_data;

typedef struct {
    xmedia_double data[MAX_ROWS][MAX_ROWS];
    xmedia_s32 row_num;
    xmedia_s32 col_num;
} r_matrix_data;

typedef struct {
    xmedia_s32 val;
    xmedia_char sql_table[MAX_TABLE_NAME_LEN];
    xmedia_s32 sql_col_num;

    xmedia_s32 matrix_row_num;
    xmedia_s32 matrix_col_num;
    xmedia_char *file_name;
    xmedia_double match_thres;
} config_info;

typedef struct {
    matrix_data *o_matrix;
    t_matrix_data *t_matrix;
    r_matrix_data *r_matrix;
} face_feature_db;

typedef struct {
    xmedia_char *add_person_name;
    xmedia_double add_person_feature[MAX_COLS];
} add_person_info;

typedef struct {
    xmedia_s32 face_num;
    xmedia_s32 match_num;
    xmedia_s32 match_col[MAX_MATCH_NUM];
    xmedia_char match_name[MAX_MATCH_NUM][MAX_NAME_LEN];
} db_match_result;

typedef struct {
    sqlite3* sql3_db;
    face_feature_db *feature_db;
    xmedia_bool use_flag;
    xmedia_bool db_empty_flag;
    xmedia_double match_thres;
    xmedia_char *filepath;
} db_attr;

xmedia_s32 svp_init_database(db_attr     **fr_db_attr, config_info *cfg);

xmedia_s32 svp_deinit_database(db_attr *fr_db_attr);

xmedia_s32 svp_get_match_result(db_attr *fr_db_attr, xmedia_svp_fr_output *face_matrix, db_match_result *result);

xmedia_s32 svp_add_someone_to_database(db_attr *fr_db_attr, add_person_info *person_info);

xmedia_s32 svp_delete_someone_from_database(db_attr *fr_db_attr, xmedia_char *name);

#ifdef __cplusplus
}
#endif

#endif
