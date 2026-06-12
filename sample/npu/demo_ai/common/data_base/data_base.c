#include "data_base.h"

static db_attr g_svp_db[MAX_DB_NUM] = { 0 };

/*
    data:
    argc: row_num
    argv: col value
    azColName: col name
*/
static xmedia_s32 name_callback(xmedia_void* data, xmedia_s32 argc, xmedia_char** argv, xmedia_char** azColName)
{
    if (data == NULL) {
        printf("callback func storage data is null\n");
        return XMEDIA_NULL;
    }

    xmedia_char *result = (xmedia_char *)data;
    if (argc > 0) {
        if (argv[0]) {
            memcpy((xmedia_char*)result, argv[0], NAME_LEN);
        }
    } else {
        printf("can not match name from index\n");
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 index_callback(xmedia_void* data, xmedia_s32 argc, xmedia_char** argv, xmedia_char** azColName)
{
    if (data == NULL) {
        printf("callback func storage data is null\n");
        return XMEDIA_NULL;
    }

    printf("come in to index_callback\n");
    xmedia_s32 *result = (xmedia_s32 *)data;
    if (argc > 0) {
        if (argv[0]) {
            *result = atoi(argv[0]);
            printf("match index: %d\n", *result);
        }
    } else {
        printf("can not match index from name\n");
    }

    return XMEDIA_SUCCESS;
}

xmedia_void printf_matrix(matrix_data *matrix) {
    printf("o_matrix row: %d, col: %d\n", matrix->row_num, matrix->col_num);

    for (xmedia_s32 i = 0; i < matrix->row_num; i++) {
        for (xmedia_s32 j = 0; j < matrix->col_num; j++) {
            printf("%lf ", matrix->data[i][j]);
        }
        printf("\n");
    }
}

xmedia_void printf_t_matrix(t_matrix_data *matrix) {
    printf("t_matrix row: %d, col: %d\n", matrix->row_num, matrix->col_num);

    for (xmedia_s32 i = 0; i < matrix->row_num; i++) {
        for (xmedia_s32 j = 0; j < matrix->col_num; j++) {
            printf("%lf ", matrix->data[i][j]);
        }
        printf("\n");
    }
}

xmedia_void printf_r_matrix(r_matrix_data *matrix) {
    printf("r_matrix row: %d, col: %d\n", matrix->row_num, matrix->col_num);

    for (int i = 0; i < matrix->row_num; i++) {
        for (int j = 0; j < matrix->col_num; j++) {
            printf("%lf ", matrix->data[i][j]);
        }
        printf("\n");
    }

}

static xmedia_s32 create_table(sqlite3* sql3_db, xmedia_s32 col_num, xmedia_char *table_name)
{
    xmedia_s32 ret;

    xmedia_char create_table_cmd[1000];
    sprintf(create_table_cmd, "CREATE TABLE IF NOT EXISTS %s (id TEXT, ", table_name);
    for (xmedia_s32 i = 1; i <= col_num - 1; i++) {
        xmedia_char columnName[20];
        sprintf(columnName, "val%d REAL", i);
        strcat(create_table_cmd, columnName);
        if (i != (col_num - 1)) {
            strcat(create_table_cmd, ", ");
        }
    }
    strcat(create_table_cmd, ");");

    printf("%s\n", create_table_cmd);

    ret = sqlite3_exec(sql3_db, create_table_cmd, 0, 0, 0);
    if (ret != SQLITE_OK) {
        printf("can not exce SQL: %s\n", sqlite3_errmsg(sql3_db));
        sqlite3_close(sql3_db);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 table_insert_one_row(sqlite3* sql3_db, xmedia_char *name, xmedia_s32 index,
    xmedia_s32 col_num, xmedia_char *table_name)
{
    xmedia_s32 ret;
    xmedia_char id[MAX_NAME_LEN];
    xmedia_char table_insert_cmd[1000]; //保证能装完513个数据 512 * (8 + 4) + 30

    sprintf(table_insert_cmd, "INSERT INTO %s (id, ", table_name);
    for (xmedia_s32 j = 1; j <= col_num - 1; j++) {
        xmedia_char col[10];
        sprintf(col, "val%d", j);
        strcat(table_insert_cmd, col);
        if (j != col_num - 1) {
            strcat(table_insert_cmd, ", ");
        }
    }

    strcat(table_insert_cmd, ") VALUES ('");

    sprintf(id, "%s", name);
    strcat(table_insert_cmd, id);
    strcat(table_insert_cmd, "', ");
    for (xmedia_s32 j = 1; j <= col_num - 1; j++) { //这里还可以增加其他信息
        xmedia_s32 value = index;
        xmedia_char value_str[20];
        sprintf(value_str, "%d", value);
        strcat(table_insert_cmd, value_str);
        if (j != col_num - 1) {
            strcat(table_insert_cmd, ", ");
        }
    }
    strcat(table_insert_cmd, ");");

    printf("insert %d info :\n %s\n", index, table_insert_cmd);

    ret = sqlite3_exec(sql3_db, table_insert_cmd, 0, 0, 0);
    if (ret != SQLITE_OK) {
        printf("can not insert SQL: %s\n", sqlite3_errmsg(sql3_db));
        sqlite3_close(sql3_db);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 search_id(sqlite3* sql3_db, xmedia_char *str, xmedia_s32 index)
{
    xmedia_s32 ret;

    if (str == NULL) {
        printf("storage name space is null\n");
        return XMEDIA_NULL;
    }

    // 外部数据
    double extra_val[SQL_COL_NUM - 1];
    for (xmedia_s32 i = 1; i <= SQL_COL_NUM - 1; i++) {
        extra_val[i - 1] = index;
    }

    // 查询匹配的行
    xmedia_char select_cmd[100];
    sprintf(select_cmd, "SELECT id FROM my_table WHERE ");
    for (xmedia_s32 i = 1; i <= SQL_COL_NUM - 1; i++) {
        xmedia_char colName[10];
        sprintf(colName, "val%d", i);
        strcat(select_cmd, colName);

        xmedia_char val_str[20];
        sprintf(val_str, " = %f", extra_val[i - 1]);
        strcat(select_cmd, val_str);
        if (i != SQL_COL_NUM - 1) {
            strcat(select_cmd, " AND ");
        }
    }
    strcat(select_cmd, ";");

    ret = sqlite3_exec(sql3_db, select_cmd, name_callback, str, 0);
    if (ret != SQLITE_OK) {
        printf("can not query SQL: %s\n", sqlite3_errmsg(sql3_db));
        sqlite3_close(sql3_db);
        return SQL_ERROR;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 search_index(sqlite3* sql3_db, xmedia_char *name_str, xmedia_s32 *index)
{
    xmedia_s32 ret;

    if (name_str == NULL) {
        printf("name is invalid\n");
        return XMEDIA_NULL;
    }

    if (index == NULL) {
        printf("storage index space is null\n");
        return XMEDIA_NULL;
    }

    // 查询匹配的行
    xmedia_char select_cmd[100];
    sprintf(select_cmd, "SELECT val1 FROM my_table WHERE id = '");
    xmedia_char col_name[50];
    sprintf(col_name, name_str);
    strcat(select_cmd, col_name);
    strcat(select_cmd, "';");

    printf("find str is :\n %s \n", select_cmd);
    ret = sqlite3_exec(sql3_db, select_cmd, index_callback, index, 0);
    if (ret != SQLITE_OK) {
        printf("can not query SQL: %s\n", sqlite3_errmsg(sql3_db));
        //sqlite3_close(sql3_db);
        return SQL_ERROR;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 insert_row(double row[], xmedia_s32 size, matrix_data *matri)
{
    if (matri == NULL) {
        printf("the matri is not exist\n");
        return XMEDIA_NULL;
    }

    if (matri->row_num >= MAX_ROWS) {
        printf("Database is full, can not insert more rows.\n");
        return PUSH_FAILED;
    }

    if (size > MAX_COLS) {
        printf("Row size exceeds maximum number of colums. can not insert rows.\n");
        return PUSH_FAILED;
    }

    for (xmedia_s32 i = 0; i < size; i++) {
        matri->data[matri->row_num][i] = row[i];
    }

    matri->row_num++;
    matri->col_num = size;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 delete_row(xmedia_s32 row_index, matrix_data *matri) {
    if (row_index < 0 || row_index > matri->row_num) {
        printf("Invalid row index. can not delete row.\n");
        return XMEDIA_FAILURE;
    }

    for (xmedia_s32 i = row_index; i < matri->row_num - 1; i++) {
        for (xmedia_s32 j = 0; j < matri->col_num; j++) {
            matri->data[i][j] = matri->data[i + 1][j];
        }
    }

    matri->row_num -= 1;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 normalize_matrix(matrix_data *matrix)
{
    if (matrix == NULL) {
        return XMEDIA_NULL;
    }

    // find the max value from matrix
    for (xmedia_s32 i = 0; i < matrix->row_num; i++) {
        double sum_of_square = 0.0;
        for (xmedia_s32 j = 0; j < matrix->col_num; j++) {
            sum_of_square += matrix->data[i][j] * matrix->data[i][j];
        }

        double model_len = sqrt(sum_of_square);
        for (xmedia_s32 j = 0; j < matrix->col_num; j++) {
            matrix->data[i][j] /= model_len;
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 matrix_transpose(matrix_data *matrix, t_matrix_data *t_matrix)
{
    if (matrix == NULL || t_matrix == NULL) {
        printf("can not transpose, matrix or t_matrix is null\n");
        return XMEDIA_NULL;
    }

    for (xmedia_s32 i = 0; i < matrix->row_num; i++) {
        for (xmedia_s32 j = 0; j < matrix->col_num; j++) {
            t_matrix->data[j][i] = matrix->data[i][j];
        }
    }

    t_matrix->row_num = matrix->col_num;
    t_matrix->col_num = matrix->row_num;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 check_dot_result(r_matrix_data *matrix, double similarity_thres, db_match_result *result)
{
    double min = 1.0;
    xmedia_s32 i, j;

    if (matrix == NULL) {
        printf("check dot result failed!\n");
        return XMEDIA_FAILURE;
    }

    // multi target, clear result firstly
    result->face_num = 0;
    result->match_num = 0;
    memset(result->match_col, -1, sizeof(xmedia_s32) * MAX_MATCH_NUM);
    for (i = 0; i < matrix->row_num; i++) {
        for (j = 0; j < matrix->col_num; j++) {
            if (matrix->data[i][j] <= similarity_thres) { // pick data from similarity_thres, data_value in [0, 1]
                if (min > matrix->data[i][j]) { // get the samllest value in row
                    min = matrix->data[i][j];
                    result->match_col[i] = j;
                }
            }
        }

        min = 1.0;
    }

    result->face_num = matrix->row_num;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 svp_matrix_dot(t_matrix_data *t_matrix, r_matrix_data *result_matrix,
    xmedia_svp_fr_output *face_matrix)
{
    if (face_matrix == NULL || t_matrix == NULL || result_matrix == NULL) {
        printf("matrix handle is null\n");
        return XMEDIA_NULL;
    }

    for (xmedia_s32 i = 0; i < face_matrix->face_num; i++) {
        for (xmedia_s32 j = 0; j < t_matrix->col_num; j++) {
            for (xmedia_s32 k = 0; k < MAX_COLS; k++) {
                result_matrix->data[i][j] += face_matrix->fr_result[i].vector[k] * t_matrix->data[k][j];
            }

            // [-1, 1] --> [0, 1]
            result_matrix->data[i][j] = 0.5 - 0.5 * result_matrix->data[i][j]; // 0.5 - 0.5*cosine
        }
    }

    result_matrix->row_num = face_matrix->face_num;
    result_matrix->col_num = t_matrix->col_num;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 check_db_cfg_info(config_info *cfg)
{
    if (cfg->file_name == NULL) {
        printf("cfg file_name is NULL\n");
        return XMEDIA_FAILURE;
    }

    if (cfg->match_thres > 1.0 || cfg->match_thres < 0.0) {
        printf("cfg match_thres %f is invaild, recommend 0.0 ~ 1.0\n", cfg->match_thres);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 check_add_person_info(add_person_info *person_info)
{
    xmedia_double sum = 0.0;
    xmedia_s32 i = 0;

    if (person_info->add_person_name == NULL) {
        printf("add person name is null\n");
        return XMEDIA_FAILURE;
    }

    for (; i < MAX_COLS; i++) {
        sum += person_info->add_person_feature[i];
    }

    if (sum == 0.0 || i != MAX_COLS) {
        printf("add person info is invalid\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 append_person_info_to_file(xmedia_char *file, add_person_info *person_info)
{
    FILE *fp;

    if (file == NULL) {
        printf("file is null\n");
        return XMEDIA_FAILURE;
    }

    fp = fopen(file, "a");
    if (fp == NULL) {
        printf("can not open file\n");
    }

    fprintf(fp, "\"%s\" #", person_info->add_person_name);
    for (xmedia_s32 i = 0; i < MAX_COLS; i++) {
        fprintf(fp, "%lf", person_info->add_person_feature[i]);
        if (i != MAX_COLS - 1) {
            fprintf(fp, ",");
        }
    }
    fprintf(fp, "#\n");

    printf("finish write data to file\n");

    fclose(fp);
    return XMEDIA_SUCCESS;
}

static xmedia_s32 delete_person_info_from_file(xmedia_char *file, xmedia_char *name)
{
    FILE *read_fp;
    FILE *clear_fp;
    FILE *write_fp;
    xmedia_char *tmp_buffer;
    xmedia_char *delete_start;
    xmedia_char *delete_end;
    xmedia_s32 file_size;

    if (file == NULL) {
        printf("file is null\n");
        return XMEDIA_FAILURE;
    }

    // read origin data to buffer
    read_fp = fopen(file, "r");
    if (read_fp == NULL) {
        printf("can not open file\n");
    }

    fseek(read_fp, 0, SEEK_END);
    file_size = ftell(read_fp);
    fseek(read_fp, 0, SEEK_SET);

    tmp_buffer = (xmedia_char*)malloc(file_size);
    if (tmp_buffer == NULL) {
        printf("malloc tmp_buffer for data file failed\n");
        return XMEDIA_FAILURE;
    }

    fread(tmp_buffer, sizeof(xmedia_char), file_size, read_fp);
    fclose(read_fp);

    // clear file
    clear_fp = fopen(file, "w");
    if (clear_fp == NULL) {
        printf("can not open clear_fp_file\n");
        goto OPEN_CLEAR_FAILED;
    }
    fclose(clear_fp);

    // determine location
    delete_start = strstr(tmp_buffer, name) - 1;
    delete_end = strchr(delete_start + MAX_NAME_LEN + 3, '#');// " 空格 # 三个字符

    write_fp = fopen(file, "w");
    if (write_fp == NULL) {
        printf("can not open write_fp_file, txt file is empty!!!\n");
        free(tmp_buffer);
        return XMEDIA_FAILURE;
    }

    fwrite(tmp_buffer, sizeof(xmedia_char), delete_start - tmp_buffer, write_fp);
    fwrite(delete_end + 1, sizeof(xmedia_char), (tmp_buffer + file_size) - (delete_end + 1), write_fp);

    fclose(write_fp);
    free(tmp_buffer);

    return XMEDIA_SUCCESS;

OPEN_CLEAR_FAILED:
    free(tmp_buffer);
    return XMEDIA_FAILURE;
}

xmedia_s32 svp_init_database(db_attr        **fr_db_attr, config_info *cfg)
{
    FILE *fp;
    xmedia_s32 ret;
    xmedia_s32 index;
    xmedia_slong file_size;
    xmedia_char *tmp_buffer;
    xmedia_char* start;
    xmedia_char *end;
    xmedia_char *sql;
    xmedia_char data[MAX_FEATURE_LEN];
    xmedia_char* token;
    xmedia_char* delimiter = ",";
    xmedia_char *err_msg;
    xmedia_s32 last_index = -1;
    xmedia_s32 match_name_index;
    xmedia_char name[MAX_NAME_LEN];
    xmedia_char *info_table = "my_table";

    if (fr_db_attr == NULL) {
        printf("fr_db_attr is null\n");
        return XMEDIA_FAILURE;
    }

    if (cfg == NULL) {
        printf("database cfg is NULL\n");
        return XMEDIA_FAILURE;
    }

    // check cfg info
    ret = check_db_cfg_info(cfg);
    if (ret != XMEDIA_SUCCESS) {
        printf("database cfg param is invalid\n");
        return XMEDIA_FAILURE;
    }

    // find idle
    for (index = 0; index < MAX_DB_NUM; index++) {
        if (!g_svp_db[index].use_flag) {
            break;
        }
    }

    if (index >= MAX_DB_NUM) {
        printf("db num is out of range 2\n");
        return XMEDIA_FAILURE;
    }

    g_svp_db[index].feature_db = malloc(sizeof(face_feature_db));
    g_svp_db[index].feature_db->o_matrix = malloc(sizeof(matrix_data));
    g_svp_db[index].feature_db->t_matrix = malloc(sizeof(t_matrix_data));
    g_svp_db[index].feature_db->r_matrix = malloc(sizeof(r_matrix_data));

    if (!g_svp_db[index].feature_db) {
        printf("g_svp_db[index].feature_db malloc failed\n");
        return MALLOC_FAILED;
    }

    if (!g_svp_db[index].feature_db->o_matrix) {
        printf("o_matrix malloc failed\n");
        free(g_svp_db[index].feature_db);
        return MALLOC_FAILED;
    }

    if (!g_svp_db[index].feature_db->t_matrix) {
        printf("t_matrix malloc failed\n");
        free(g_svp_db[index].feature_db);
        free(g_svp_db[index].feature_db->o_matrix);
        return MALLOC_FAILED;
    }

    if (!g_svp_db[index].feature_db->r_matrix) {
        printf("r_matrix malloc failed\n");
        free(g_svp_db[index].feature_db);
        free(g_svp_db[index].feature_db->o_matrix);
        free(g_svp_db[index].feature_db->t_matrix);
        return MALLOC_FAILED;
    }

    memset(g_svp_db[index].feature_db->o_matrix, 0x00, sizeof(matrix_data));
    memset(g_svp_db[index].feature_db->t_matrix, 0x00, sizeof(t_matrix_data));
    memset(g_svp_db[index].feature_db->r_matrix, 0x00, sizeof(r_matrix_data));

    g_svp_db[index].match_thres = cfg->match_thres;
    g_svp_db[index].filepath = cfg->file_name;

    ret = sqlite3_open("./res/person_name_id", &g_svp_db[index].sql3_db);
    if (ret != SQLITE_OK) {
        printf("can not open SQL database: %s\n", sqlite3_errmsg(g_svp_db[index].sql3_db));
        return 1;
    }

    ret = create_table(g_svp_db[index].sql3_db, SQL_COL_NUM, info_table);
    if (ret != 0) {
        printf("can not create table\n");
        return XMEDIA_FAILURE;
    }

    fp = fopen(cfg->file_name, "r");
    if (!fp) {
        printf("open data file failed\n");
        return XMEDIA_FAILURE;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    tmp_buffer = (xmedia_char*)malloc(file_size);
    if (tmp_buffer == NULL) {
        printf("malloc tmp_buffer for data file failed\n");
        goto MALLOC_TMP_BUFF;
    }

    fread(tmp_buffer, sizeof(xmedia_char), file_size, fp);

    start = strchr(tmp_buffer, '"');
    while (start != NULL) {
        end = strchr(start + 1, '"');
        strncpy(name, start + 1, end - start - 1);
        name[end - start - 1] = '\0';
        printf("name: %s\n", name);

        // 查找当前数据库中最大的index
        sql = "SELECT MAX(val1) FROM my_table;";
        ret = sqlite3_exec(g_svp_db[index].sql3_db, sql, index_callback, &last_index, &err_msg);
        if (ret != SQLITE_OK) {
            sqlite3_free(err_msg);
            sqlite3_close(g_svp_db[index].sql3_db);
            return 1;
        }
        printf("sql table max index %d\n", last_index);

        // 判断数据库中是否有这个人
        match_name_index = -1;
        ret = search_index(g_svp_db[index].sql3_db, name, &match_name_index);
        if (ret != XMEDIA_SUCCESS) {
            printf("can not search sql\n");
            goto FAIL_INSERT;
        }

        if (match_name_index == -1) { // add new one
            printf("sql db cann't find %s, add!!!\n", name);
            ret =  table_insert_one_row(g_svp_db[index].sql3_db, name, last_index + 1, SQL_COL_NUM, info_table);
            if (ret != 0) {
                printf("insert info data failed\n");
                goto FAIL_INSERT;
            }
        }

        // 提取##之间的数据
        start = strchr(end + 1, '#');
        end = strchr(start + 1, '#');
        strncpy(data, start + 1, end - start - 1);
        data[end - start - 1] = '\0';

        token = strtok(data, delimiter);
        xmedia_double row[MAX_COLS] = {0};
        xmedia_s32 data_index = 0;
        while (token != NULL) {
            if (data_index > MAX_COLS) {
               goto FAIL_INSERT;
            }

            row[data_index] = atof(token);
            token = strtok(NULL, delimiter);
            data_index++;
        }

        ret = insert_row(row, MAX_COLS, g_svp_db[index].feature_db->o_matrix);
        if (ret != 0) {
           printf("insert feature data failed\n");
           goto FAIL_INSERT;
        }

        printf("finish one people\n");

        start = strchr(end + 1, '"');
    }

    fclose(fp);
    free(tmp_buffer);

    // txt is empty
    if (g_svp_db[index].feature_db->o_matrix->row_num == 0) {
        printf("txt file is empty\n");
        g_svp_db[index].db_empty_flag = XMEDIA_TRUE;
    }

    // normlize
    ret = normalize_matrix(g_svp_db[index].feature_db->o_matrix);
    if (ret != XMEDIA_SUCCESS) {
        printf("normlize failed\n");
        goto FAIL_INSERT;
    }

    ret = matrix_transpose(g_svp_db[index].feature_db->o_matrix, g_svp_db[index].feature_db->t_matrix);
    if (ret != XMEDIA_SUCCESS) {
        printf("matrix_transpose failed\n");
        goto FAIL_INSERT;
    }

    g_svp_db[index].use_flag = XMEDIA_TRUE;
    *fr_db_attr = &g_svp_db[index];

    printf("database init successfully\n");

    return XMEDIA_SUCCESS;

FAIL_INSERT:
    free(tmp_buffer);
MALLOC_TMP_BUFF:
    free(g_svp_db[index].feature_db->o_matrix);
    free(g_svp_db[index].feature_db->t_matrix);
    free(g_svp_db[index].feature_db->r_matrix);
    free(g_svp_db[index].feature_db);

    sqlite3_close(g_svp_db[index].sql3_db);

    fclose(fp);
    return XMEDIA_FAILURE;
}

xmedia_s32 svp_deinit_database(db_attr *fr_db_attr)
{
    xmedia_s32 ret;
    xmedia_s32 db_handle = 0;

    if (fr_db_attr == NULL) {
        printf("fr_db_attr is null\n");
        return XMEDIA_FAILURE;
    }

    for (; db_handle < MAX_DB_NUM; db_handle++) {
        if (fr_db_attr == &g_svp_db[db_handle]) {
            break;
        }
    }

    if (db_handle >= MAX_DB_NUM) {
        return XMEDIA_FAILURE;
    }

    if (!g_svp_db[db_handle].use_flag) {
        return XMEDIA_FAILURE;
    }

    if (g_svp_db[db_handle].feature_db->o_matrix) {
        free(g_svp_db[db_handle].feature_db->o_matrix);
    }

    if (g_svp_db[db_handle].feature_db->t_matrix) {
        free(g_svp_db[db_handle].feature_db->t_matrix);
    }

    if (g_svp_db[db_handle].feature_db->r_matrix) {
        free(g_svp_db[db_handle].feature_db->r_matrix);
    }

    if (g_svp_db[db_handle].feature_db) {
        free(g_svp_db[db_handle].feature_db);
    }

    if (g_svp_db[db_handle].sql3_db) {
        ret = sqlite3_close(g_svp_db[db_handle].sql3_db);
        if (ret != SQLITE_OK) {
            printf("sql close failed\n");
        }
    }

    g_svp_db[db_handle].use_flag = XMEDIA_FALSE;

    printf("database deinit successfully\n");

    return 0;
}

xmedia_s32 svp_get_match_result(db_attr *fr_db_attr, xmedia_svp_fr_output *face_matrix, db_match_result *result)
{
    xmedia_s32 ret;
    xmedia_s32 db_handle = 0;

    if (result == NULL) {
        printf("db_result is null\n");
        return XMEDIA_FAILURE;
    }

    if (face_matrix == NULL) {
        printf("face matrix is null\n");
        return XMEDIA_FAILURE;
    }

    if (face_matrix->face_num == 0) {
       return XMEDIA_SUCCESS;
    }

    if (fr_db_attr == NULL) {
        printf("db handle is null\n");
        return XMEDIA_FAILURE;
    }

    for (; db_handle < MAX_DB_NUM; db_handle++) {
        if (fr_db_attr == &g_svp_db[db_handle]) {
            break;
        }
    }

    if (db_handle >= MAX_DB_NUM) {
        printf("db_handle is invalid\n");
        return XMEDIA_FAILURE;
    }

    if (fr_db_attr->db_empty_flag) {
        printf("data_base is empty, please input data\n");
        return XMEDIA_SUCCESS;
    }
    t_matrix_data *t_matrix = g_svp_db[db_handle].feature_db->t_matrix;
    r_matrix_data *result_matrix = g_svp_db[db_handle].feature_db->r_matrix;
    memset(result_matrix, 0x00, sizeof(r_matrix_data));
    ret = svp_matrix_dot(t_matrix, result_matrix, face_matrix);
    if (ret != XMEDIA_SUCCESS) {
        printf("matrix dot failed\n");
        return XMEDIA_FAILURE;
    }
    // check result
    memset(result->match_name, 0, sizeof(result->match_name));
    check_dot_result(result_matrix, fr_db_attr->match_thres, result);
    for (xmedia_s32 i = 0; i < result->face_num; i++) {
        if (result->match_col[i] != -1) {
            ret = search_id(fr_db_attr->sql3_db,result->match_name[i], result->match_col[i]);
            if (ret != 0) { // got name
                printf("DB can not search \n");
            }

            if (strcmp(result->match_name[i], "") != 0) { // got name
                result->match_num++;
//                printf("DB match %s\n", result->match_name[i]);
            }
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 svp_add_someone_to_database(db_attr *fr_db_attr, add_person_info *person_info)
{
    xmedia_s32 ret = 0;
    xmedia_char *sql;
    xmedia_char *err_msg;
    xmedia_s32 last_index = -1;
    xmedia_s32 match_name_index;
    xmedia_char *info_table = "my_table";
    xmedia_s32 db_handle = 0;

    if (fr_db_attr == NULL) {
        printf("fr_db_attr is null\n");
        return XMEDIA_FAILURE;
    }

    if (person_info == NULL) {
        printf("person info is null\n");
        return XMEDIA_FAILURE;
    }

    for (; db_handle < MAX_DB_NUM; db_handle++) {
        if (fr_db_attr == &g_svp_db[db_handle]) {
            break;
        }
    }

    if (db_handle >= MAX_DB_NUM) {
        return XMEDIA_FAILURE;
    }

    // check person info
    ret = check_add_person_info(person_info);
    if (ret != XMEDIA_SUCCESS) {
        printf("person info invalid\n");
        return XMEDIA_FAILURE;
    }

    // find max index in sql
    sql = "SELECT MAX(val1) FROM my_table;";
    ret = sqlite3_exec(fr_db_attr->sql3_db, sql, index_callback, &last_index, &err_msg);
    if (ret != SQLITE_OK) {
        sqlite3_free(err_msg);
        sqlite3_close(fr_db_attr->sql3_db);
        return XMEDIA_FAILURE;
    }
    printf("sql table max index %d\n", last_index);

    // check if has add person
    match_name_index = -1;
    ret = search_index(fr_db_attr->sql3_db, person_info->add_person_name, &match_name_index);
    if (ret != XMEDIA_SUCCESS) {
        printf("can not search sql\n");
        return XMEDIA_FAILURE;
    }

    // add to sql
    if (match_name_index == -1) {
        ret = table_insert_one_row(fr_db_attr->sql3_db, person_info->add_person_name,
                                   last_index + 1, SQL_COL_NUM, info_table);
        if (ret != 0) {
            printf("insert info data failed\n");
            return XMEDIA_FAILURE;
        }

        // add to feature o_matrix
        ret = insert_row(person_info->add_person_feature, MAX_COLS, fr_db_attr->feature_db->o_matrix);
        if (ret != 0) {
            sqlite3_free(err_msg);
            sqlite3_close(fr_db_attr->sql3_db);
            return XMEDIA_FAILURE;
        }
        fr_db_attr->db_empty_flag = XMEDIA_FALSE;

        ret = normalize_matrix(fr_db_attr->feature_db->o_matrix);
        if (ret != XMEDIA_SUCCESS) {
            sqlite3_free(err_msg);
            sqlite3_close(fr_db_attr->sql3_db);
            return XMEDIA_FAILURE;
        }

        ret = matrix_transpose(fr_db_attr->feature_db->o_matrix, fr_db_attr->feature_db->t_matrix);
        if (ret != XMEDIA_SUCCESS) {
            sqlite3_free(err_msg);
            sqlite3_close(fr_db_attr->sql3_db);
            return XMEDIA_FAILURE;
        }

        // update info to txt file
        ret = append_person_info_to_file(fr_db_attr->filepath, person_info);
        if (ret != XMEDIA_SUCCESS) {
            printf("append_person_info_to_file\n");
            return XMEDIA_FAILURE;
        }

        printf("add %s to database successfully, index is %d\n", person_info->add_person_name, last_index + 1);
        return XMEDIA_SUCCESS;
    }

    printf("person already in database\n");
    return XMEDIA_FAILURE;
}

xmedia_s32 svp_delete_someone_from_database(db_attr *fr_db_attr, xmedia_char *name)
{
    xmedia_s32 ret;
    xmedia_s32 match_name_index;
    xmedia_char *err_msg;
    xmedia_s32 db_handle = 0;

    if (fr_db_attr == NULL) {
        printf("db handle is null\n");
        return XMEDIA_FAILURE;
    }

    for (; db_handle < MAX_DB_NUM; db_handle++) {
        if (fr_db_attr == &g_svp_db[db_handle]) {
            break;
        }
    }

    if (db_handle >= MAX_DB_NUM) {
        return XMEDIA_FAILURE;
    }

    if (name == NULL) {
        printf("delete name is invalid\n");
        return XMEDIA_FAILURE;
    }

    // find index by name
    match_name_index = -1;
    ret = search_index(fr_db_attr->sql3_db, name, &match_name_index);
    if (ret != XMEDIA_SUCCESS) {
        printf("can not search sql\n");
        return XMEDIA_FAILURE;
    }

    // if index valid, go on
    if (match_name_index != -1) {
        // updata sql index
        xmedia_char delete_table_cmd[100];
        sprintf(delete_table_cmd, "DELETE FROM my_table WHERE val1 = %d;", match_name_index);
        ret = sqlite3_exec(fr_db_attr->sql3_db, delete_table_cmd, NULL, 0, &err_msg);
        if (ret != SQLITE_OK) {
            sqlite3_free(err_msg);
            sqlite3_close(fr_db_attr->sql3_db);
            return XMEDIA_FAILURE;
        }

        xmedia_char update_table_cmd[100];
        sprintf(update_table_cmd, "UPDATE my_table SET val1 = val1 - 1 WHERE val1 > %d;", match_name_index);
        ret = sqlite3_exec(fr_db_attr->sql3_db, update_table_cmd, NULL, 0, &err_msg);
        if (ret != SQLITE_OK) {
            sqlite3_free(err_msg);
            sqlite3_close(fr_db_attr->sql3_db);
            return XMEDIA_FAILURE;
        }

        // delete feature o_matrix row by index
        ret = delete_row(match_name_index, fr_db_attr->feature_db->o_matrix);
        if (ret != XMEDIA_SUCCESS) {
            printf("use name search index failed\n");
            return XMEDIA_FAILURE;
        }

        ret = normalize_matrix(fr_db_attr->feature_db->o_matrix);
        if (ret != XMEDIA_SUCCESS) {
            sqlite3_free(err_msg);
            sqlite3_close(fr_db_attr->sql3_db);
            return XMEDIA_FAILURE;
        }

        ret = matrix_transpose(fr_db_attr->feature_db->o_matrix, fr_db_attr->feature_db->t_matrix);
        if (ret != XMEDIA_SUCCESS) {
            sqlite3_free(err_msg);
            sqlite3_close(fr_db_attr->sql3_db);
            return XMEDIA_FAILURE;
        }

        // delete person from txt file
        ret = delete_person_info_from_file(fr_db_attr->filepath, name);
        if (ret != XMEDIA_SUCCESS) {
            printf("delete_person_info_from_file failed\n");
            return XMEDIA_FAILURE;
        }

        printf("delete %s from database successfully,origin index is %d\n", name, match_name_index);
        return XMEDIA_SUCCESS;
    }

    printf("%s not in database\n", name);
    return XMEDIA_FAILURE;
}
