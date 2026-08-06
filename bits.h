/*

Small convenience macros and utilities.

*/

// A parallel fprintf that only prints from rank 0

#pragma once

#define mfprintf(...) do { if (rank == 0) fprintf(__VA_ARGS__); } while(0)

// A parallel fprintf that prepends the rank to the output

#define Mfprintf(...) do { fprintf(stderr, "[rank %d] ", rank); fprintf(__VA_ARGS__); } while(0)