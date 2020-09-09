#include "headers/fn_sequential_x86_42pages-heap-21-20-10000-config.h"
#include "benchmark.h"

static const int g_row[] = {row_fn_sequential_x86_42pages};
static const int g_col[] = {col_fn_sequential_x86_42pages};
static struct interval *g_pages_interval[] = {pages_interval_fn_sequential_x86_42pages};
static unsigned *g_trials[] = {trials_fn_sequential_x86_42pages};
static unsigned *g_work[] = {work_fn_sequential_x86_42pages};

struct workload fn_sequential_x86_42pages_21x_20y_10000red = {
.size = 1,
.row = g_row,
.col = g_col,
.pages_interval = g_pages_interval,
.trials = g_trials,
.work = g_work
};
