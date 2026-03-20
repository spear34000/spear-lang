#ifndef SPEARC_STRING_LIST_H
#define SPEARC_STRING_LIST_H

static bool string_list_has(StringList *list, const char *value) {
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->items[i], value) == 0) {
            return true;
        }
    }
    return false;
}

static void string_list_add(StringList *list, const char *value) {
    if (string_list_has(list, value)) {
        return;
    }
    if (list->count == list->cap) {
        size_t next = list->cap ? list->cap * 2 : 8;
        list->items = realloc(list->items, next * sizeof(char *));
        if (!list->items) {
            fprintf(stderr, "%s: %s\n", compiler_message("error_prefix"), compiler_message("oom"));
            exit(1);
        }
        list->cap = next;
    }
    list->items[list->count++] = xstrdup(value);
}

#endif
