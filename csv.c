#include "csv.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define csv_malloc(size)    malloc(size)
#define csv_free(ptr)       free(ptr)
#define csv_print(s, ...)   printf(s, __VA_ARGS__)

static void get_dimensions(FILE* fptr, int* num_rows, int* num_cols)
{
    char ch;
    int nr, nc, c;
    nr = nc = c = 0;
    while ((ch = fgetc(fptr)) != EOF) {
        if (ch == ',') {
            c += 1;
        }
        else if (ch == '\n') {
            nc = (c+1 > nc) ? c+1 : nc;
            c = 0;
            nr += 1;
        }
    }
    *num_rows = nr;
    *num_cols = nc;
}

// 0 = empty, 1 = int, 2 = float, 3 = string
static int substr_type(int n, const char* s)
{
    if (n == 0) 
        return 0;
    int res = 1;
    for (int i = 0; i < n; i++) {
        if (!isdigit(s[i])) {
            if (s[i] == '.')
                res = 2;
            else
                return 3;
        }
    }
    return res;
}

static Cell read_next_cell(FILE* fptr)
{
    Cell cell;
    int i, len, type;
    char c;
    char* substr;
    long long start;

    start = ftell(fptr);

    len = 0;

    do { 
        c = fgetc(fptr);
        len++;
    } while (c != ',' && c != '\n' && c != EOF);

    substr = csv_malloc(len * sizeof(char));

    fseek(fptr, start, SEEK_SET);

    for (i = 0; i < len; i++)
        substr[i] = fgetc(fptr);
    substr[len-1] = '\0';

    type = substr_type(len-1, substr);
    if (type == 0) {
        cell.type = CSV_EMPTY;
        csv_free(substr);
    } else if (type == 1) {
        cell.type = CSV_INT;
        cell.val_int = atol(substr);
        csv_free(substr);
    } else if (type == 2) {
        cell.type = CSV_FLOAT;
        cell.val_float = atof(substr);
        csv_free(substr);
    } else {
        cell.type = CSV_STRING;
        cell.val_string = substr;
    }

    return cell;
}

CSV* csv_read(const char* path)
{
    FILE* fptr;
    CSV* csv;
    Cell* cells;
    int num_rows, num_cols;
    int row, col;

    csv = NULL;

    fptr = fopen(path, "rb");
    if (fptr == NULL)
        return NULL;

    get_dimensions(fptr, &num_rows, &num_cols);

    fseek(fptr, SEEK_SET, 0);

    cells = csv_malloc(num_rows * num_cols * sizeof(Cell));
    for (row = 0; row < num_rows; row++)
        for (col = 0; col < num_cols; col++)
            cells[row * num_cols + col] = read_next_cell(fptr);

    fclose(fptr);

    csv = csv_malloc(sizeof(CSV));
    csv->num_rows = num_rows;
    csv->num_cols = num_cols;
    csv->cells = cells;

    return csv;
}

void csv_write(CSV* csv, const char* path)
{
    FILE* fptr;
    Cell cell;
    int row, col;

    fptr = fopen(path, "w");
    if (fptr == NULL)
        return;

    for (row = 0; row < csv->num_rows; row++) {
        for (col = 0; col < csv->num_cols; col++) {
            cell = *csv_cell(csv, row, col);
            if (cell.type == CSV_EMPTY)
                ;
            else if (cell.type == CSV_INT)
                fprintf(fptr, "%lld", cell.val_int);
            else if (cell.type == CSV_FLOAT)
                fprintf(fptr, "%f", cell.val_float);
            else
                fprintf(fptr, cell.val_string);
            fprintf(fptr, ((col+1) % csv->num_cols == 0) ? "\n" : ",");
        }
    }

    fflush(fptr);
    fclose(fptr);
}

void csv_destroy(CSV* csv)
{
    for (int i = 0; i < csv->num_rows * csv->num_cols; i++)
        if (csv->cells[i].type == CSV_STRING)
            csv_free(csv->cells[i].val_string);
    csv_free(csv->cells);
    csv_free(csv);
}

CSVint* csv_column_int(CSV* csv, int col, int row_start, int row_end)
{
    CSVint* arr;
    Cell cell;
    int row;

    if (row_start < 0 || row_end < 0)
        return NULL;
    if (row_start >= csv->num_rows || row_end >= csv->num_rows)
        return NULL;
    if (row_start > row_end)
        return NULL;

    arr = csv_malloc((row_end-row_start+1) * sizeof(CSVint));

    for (row = row_start; row <= row_end; row++) {
        cell = *csv_cell(csv, row, col);
        if (cell.type != CSV_INT) {
            csv_free(arr);
            return NULL;
        }
        arr[row-row_start] = cell.val_int;
    }

    return arr;
}

CSVfloat* csv_column_float(CSV* csv, int col, int row_start, int row_end)
{
    CSVfloat* arr;
    Cell cell;
    int row;

    if (row_start < 0 || row_end < 0)
        return NULL;
    if (row_start >= csv->num_rows || row_end >= csv->num_rows)
        return NULL;
    if (row_start > row_end)
        return NULL;

    arr = csv_malloc((row_end-row_start+1) * sizeof(CSVfloat));

    for (row = row_start; row <= row_end; row++) {
        cell = *csv_cell(csv, row, col);
        if (cell.type == CSV_INT)
            arr[row-row_start] = (CSVfloat)cell.val_int;
        else if (cell.type == CSV_FLOAT)
            arr[row-row_start] = cell.val_float;
        else {
            csv_free(arr);
            return NULL;
        }
    }

    return arr;
}

char** csv_column_string(CSV* csv, int col, int row_start, int row_end)
{
    char** arr;
    Cell cell;
    int row, n;
    char buf[64];
    char* string;

    if (row_start < 0 || row_end < 0)
        return NULL;
    if (row_start >= csv->num_rows || row_end >= csv->num_rows)
        return NULL;
    if (row_start > row_end)
        return NULL;

    arr = csv_malloc((row_end-row_start+1) * sizeof(char*));

    for (row = row_start; row <= row_end; row++) {
        cell = *csv_cell(csv, row, col);
        if (cell.type == CSV_EMPTY) {
            arr[row-row_start] = "";
        } else if (cell.type == CSV_INT) {
            sprintf(buf, "%lld", cell.val_int);
            n = strlen(buf);
            string = csv_malloc((n+1) * sizeof(char));
            strncpy(string, buf, n+1);
            arr[row-row_start] = string;
        } else if (cell.type == CSV_FLOAT) {
            sprintf(buf, "%f", cell.val_float);
            n = strlen(buf);
            string = csv_malloc((n+1) * sizeof(char));
            strncpy(string, buf, n+1);
            arr[row-row_start] = string;
        } else {
            n = strlen(cell.val_string);
            string = csv_malloc((n+1) * sizeof(char));
            strncpy(string, cell.val_string, n+1);
            arr[row-row_start] = string;
        }
    }

    return arr;
}

int csv_num_rows(CSV* csv)
{
    return csv->num_rows;
}

int csv_num_cols(CSV* csv)
{
    return csv->num_cols;
}

Cell* csv_cell(CSV* csv, int row, int col)
{
    return &csv->cells[row * csv->num_cols + col];
}

CSVEnum csv_type(CSV* csv, int row, int col)
{
    return csv->cells[row * csv->num_cols + col].type;
}

CSVint csv_int(CSV* csv, int row, int col)
{
    return csv->cells[row * csv->num_cols + col].val_int;
}

CSVfloat csv_float(CSV* csv, int row, int col)
{
    return csv->cells[row * csv->num_cols + col].val_float;
}

char* csv_string(CSV* csv, int row, int col)
{
    return csv->cells[row * csv->num_cols + col].val_string;
}

CSVEnum csv_cell_type(Cell* cell)
{
    return cell->type;
}

CSVint csv_cell_int(Cell* cell)
{
    return cell->val_int;
}

CSVfloat csv_cell_float(Cell* cell)
{
    return cell->val_float;
}

char* csv_cell_string(Cell* cell)
{
    return cell->val_string;
}

