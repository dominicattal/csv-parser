#ifndef CSV_H
#define CSV_H

typedef long long   CSVint;
typedef double      CSVfloat;

typedef enum {
    CSV_EMPTY, 
    CSV_INT,
    CSV_FLOAT,
    CSV_STRING
} CSVEnum;

typedef struct {
    CSVEnum type;
    union {
        CSVint val_int;
        CSVfloat val_float;
        char* val_string;
    };
} Cell;

typedef struct {
    int num_rows;
    int num_cols;
    Cell* cells;
} CSV;

// Object creation/deletion
CSV*        csv_read(const char* path);
void        csv_write(CSV* csv, const char* path);
void        csv_destroy(CSV* csv);

// CSV Queries
// does no type checking
int         csv_num_rows(CSV* csv);
int         csv_num_cols(CSV* csv);
Cell*       csv_cell(CSV* csv, int row, int col);
CSVEnum     csv_type(CSV* csv, int row, int col);
CSVint      csv_int(CSV* csv, int row, int col);
CSVfloat    csv_float(CSV* csv, int row, int col);
char*       csv_string(CSV* csv, int row, int col);

// Cell Queries
// does no type checking
CSVEnum     csv_cell_type(Cell* cell);
CSVint      csv_cell_int(Cell* cell);
CSVfloat    csv_cell_float(Cell* cell);
char*       csv_cell_string(Cell* cell);

// CSV operations
// returns (row_end - row_start + 1) sized array of type, 0-indexed
// returns NULL if row_start or row end is out of bounds

// returns NULL if number is not an int
CSVint*     csv_column_int(CSV* csv, int col, int row_start, int row_end);

// returns NULL if number is not an int or float
CSVfloat*   csv_column_float(CSV* csv, int col, int row_start, int row_end);

// works for all types
// each string is copied. you are responsible for freeing memory
char**      csv_column_string(CSV* csv, int col, int row_start, int row_end);

#endif
