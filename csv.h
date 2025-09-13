#ifndef CSV_H
#define CSV_H

typedef enum {
    CSV_INT,
    CSV_FLOAT,
    CSV_STRING
} CSVEnum;

typedef struct {
    CSVEnum type;
    union {
        long long val_int;
        double val_float;
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
int         csv_num_rows(CSV* csv);
int         csv_num_cols(CSV* csv);
Cell*       csv_cell(CSV* csv, int row, int col);
CSVEnum     csv_type(CSV* csv, int row, int col);
long long   csv_int(CSV* csv, int row, int col);
double      csv_float(CSV* csv, int row, int col);
char*       csv_string(CSV* csv, int row, int col);

// Cell Queries
CSVEnum     cell_type(Cell* cell);
long long   cell_int(Cell* cell);
double      cell_float(Cell* cell);
char*       cell_string(Cell* cell);

#endif
