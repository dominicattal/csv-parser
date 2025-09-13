#include "csv.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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

// 0 = int, 1 = float, 2 = string
static int substr_type(int n, const char* s)
{
    int res = 0;
    for (int i = 0; i < n; i++) {
        if (!isdigit(s[i])) {
            if (s[i] == '.')
                res = 1;
            else
                return 2;
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
        cell.type = CSV_INT;
        cell.val_int = atol(substr);
        csv_free(substr);
    } else if (type == 1) {
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
    char c;

    csv = NULL;

    fptr = fopen(path, "rb");
    if (fptr == NULL)
        return NULL;

    get_dimensions(fptr, &num_rows, &num_cols);
    csv_print("%d %d\n", num_rows, num_cols);
    fseek(fptr, SEEK_SET, 0);

    cells = csv_malloc(num_rows * num_cols * sizeof(Cell));
    for (row = 0; row < num_rows; row++)
        for (col = 0; col < num_cols; col++)
            cells[row * num_cols + col] = read_next_cell(fptr);


    csv = csv_malloc(sizeof(CSV));
    csv->num_rows = num_rows;
    csv->num_cols = num_cols;
    csv->cells = cells;

    return csv;
}

void csv_write(CSV* csv, const char* path)
{
}

void csv_destroy(CSV* csv)
{
    for (int i = 0; i < csv->num_rows * csv->num_cols; i++)
        if (csv->cells[i].type == CSV_STRING)
            csv_free(csv->cells[i].val_string);
    csv_free(csv->cells);
    csv_free(csv);
}

int csv_num_rows(CSV* csv)
{
}

int csv_num_cols(CSV* csv)
{
}

Cell* csv_cell(CSV* csv, int row, int col)
{

}

CSVEnum csv_type(CSV* csv, int row, int col)
{
}

long long csv_int(CSV* csv, int row, int col)
{
}

double csv_float(CSV* csv, int row, int col)
{
}

char* csv_string(CSV* csv, int row, int col)
{
}

CSVEnum cell_type(Cell* cell)
{
}

long long cell_int(Cell* cell)
{
}

double cell_float(Cell* cell)
{
}

char* cell_string(Cell* cell)
{
}

