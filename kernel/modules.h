#pragma once

#include "stivale2.h"

typedef void *entry_fn_t();

void print_modules(struct stivale2_struct *hdr);

void load_init(struct stivale2_struct *hdr);

void *load_module(struct stivale2_struct *hdr, char *str);