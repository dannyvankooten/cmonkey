/*-
 * Copyright (c) 2019 Abhinav Upadhyay <er.abhinav.upadhyay@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <string.h>

#include "cmonkey_utils.h"
#include "test_utils.h"

static void
test_hash_table_init(void)
{
    print_test_separator_line();
    printf("Testing hash table init\n");
    cm_hash_table *table = cm_hash_table_init(string_hash_function,
        string_equals, NULL, NULL);
    test(table != NULL, "Failed to allocate hash table\n");
    test(table->table_size == INITIAL_HASHTABLE_SIZE,
        "Expected hash table to initialize to size %d, found size %zu\n",
        INITIAL_HASHTABLE_SIZE, table->table_size);
    for (size_t i = 0; i < INITIAL_HASHTABLE_SIZE; i++)
        test(table->table[i] == NULL,
            "Expected all entries of the table to be NULL, %zu entry is not null\n",
            i);
    test(table->used_slots->length == 0, "Expected nentries field to be 0, found %zu\n",
        table->used_slots->length);
    test(table->nkeys == 0, "Expected nkeys to be 0, found %zu\n", table->nkeys);
    cm_hash_table_free(table);
}

static void
test_hash_table_put(void)
{
    typedef struct {
        void *key;
        void *value;
    } test_data;

    test_data data[] = {
        {(void *) "key1", (void *) "value1"},
        {(void *) "key2", (void *) "value2"},
        {(void *) "key3", (void *) "value3"}
    };
    print_test_separator_line();
    printf("Testing hash table get and put\n");
    cm_hash_table *table = cm_hash_table_init(string_hash_function,
        string_equals, free, free);
    size_t ndatapoints = sizeof(data) / sizeof(data[0]);
    for (size_t i = 0; i < ndatapoints; i++) {
        test_data d = data[i];
        cm_hash_table_put(table, strdup((char *)d.key), strdup((char *)d.value));
    }

    test(table->nkeys == ndatapoints, "Expected nkeys to be %zu, found %zu\n",
        ndatapoints, table->nkeys);
    
    for (size_t i = 0; i < ndatapoints; i++) {
        test_data d = data[i];
        void *value = cm_hash_table_get(table, d.key);
        test(value != NULL, "Expected non-NULL value for key %s\n", (char *)d.key);
        test(strcmp((char *) value, (char *) d.value) == 0,
            "Expected value for key %s: %s, found %s\n",
            (char *)d.key, (char *)d.value, (char *)value);
    }

    void *value = cm_hash_table_get(table, (void *)"apple");
    test(value == NULL, "Expected value for key apple to be NULL\n");
    cm_hash_table_free(table);
}

static void
test_cm_array_list_init(void)
{
    print_test_separator_line();
    printf("Testing array list init\n");
    cm_array_list *list = cm_array_list_init(5, free);
    test(list->array_size == 5, "Expected array list size 5, found %zu\n", list->array_size);
    test(list->length == 0, "Expected array list length 0, found %zu\n", list->length);
    cm_array_list_free(list);
    printf("Array list init test passed\n");
}

static void
test_cm_array_list_init_size_t(void)
{
    print_test_separator_line();
    printf("Testing array list init for size_t\n");
    cm_array_list *list = cm_array_list_init_size_t(2, 2, 0, 1);
    test(list->array_size == 2, "Expected array list size 2, found %zu\n", list->array_size);
    test(list->length == 2, "Expected array list length 2, found %zu\n", list->length);
    size_t value = (size_t) cm_array_list_get(list, 0);
    test(value == 0, "Expected value at index 0 to be 0, found %zu\n", value);
    value = (size_t) cm_array_list_get(list, 1);
    test(value == 1, "Expected value at index 1 to be 1, found %zu\n", value);
    cm_array_list_free(list);
    printf("Array list init test passed\n");
}

static void
test_cm_array_list(void)
{
    print_test_separator_line();
    printf("Testing array list add\n");
    cm_array_list *list = cm_array_list_init(2, &free);
    int ret = cm_array_list_add(list, strdup("first"));
    test(ret == 1, "list add failed with return value %d\n", ret);
    test(list->length == 1,"Expected list length 1, found %zu\n", list->length);
    test(list->array_size == 2, "Expected list array size 2, found %zu\n", list->array_size);
    test(strcmp((char *) list->array[0], "first") == 0,
        "Expected object value at index 0 first, found %s\n", (char *)list->array[0]);

    ret = cm_array_list_add_at(list, 1, "second");
    test(ret == 0, "list add at index 1 did not fail\n");

    ret = cm_array_list_add(list, strdup("second"));
    test(ret == 1, "list add failed with return value %d\n", ret);
    test(list->length == 2, "Expected list length 2, found %zu\n", list->length);
    test(list->array_size == 2, "Expected list array size 2, found %zu\n", list->array_size);
    test(strcmp((char *)list->array[1], "second") == 0,
        "Expected second value \"second\", found \"%s\"\n", (char *)list->array[1]);

    ret = cm_array_list_add(list, strdup("third"));
    test(ret == 1, "list add at index 2 failed with return value %d\n", ret);
    test(list->length == 3, "Expected list length 3, found %zu\n", list->length);
    test(list->array_size == 5, "Expected list array size 5, found %zu\n", list->array_size);
    test(strcmp((char *)list->array[2], "third") == 0, "Expected third value \"third\", found \"%s\"\n", (char *)list->array[2]);

    ret = cm_array_list_add_at(list, 1, strdup("new second"));
    test(ret == 1, "Expected list add to pass at index 1, it failed with return value %d\n", ret);
    test(list->length == 3, "Expected list length 3, found %zu\n", list->length);
    test(list->array_size == 5, "Expected list array size 5, found %zu\n", list->array_size);
    test(strcmp((char *)list->array[1], "new second") == 0, "Expected third value \"third\", found \"%s\"\n", (char *)list->array[1]);


    void *value = cm_array_list_get(list, 1);
    test(strcmp((char *) value, "new second") == 0,
        "Expected value at second index as \"new second\", got \"%s\"\n", (char *) value);
    
    value = cm_array_list_first(list);
    test(strcmp((char *) value, "first") == 0,
        "Expected first value \"first\", got \"%s\"\n", (char *)value);
    
    value = cm_array_list_last(list);
    test(strcmp((char *)value, "third") == 0, "Expected last value \"third\", got \"%s\"\n", (char *) value);

    value = cm_array_list_get(list, 10);
    test(value == NULL, "Expected get to fail at index 10\n");

    cm_array_list_remove(list, 1);
    test(list->length == 2,
        "Expected list length 2 after removing one element, got %zu\n",
        list->length);
    test(list->array_size == 2,
        "Expected list array size 2 after element removal, got %zu\n",
        list->array_size);
    test(strcmp((char *)list->array[0], "first") == 0, "Found value \"%s\" at first index after removal\n", (char *)list->array[0]);
    test(strcmp((char *)list->array[1], "third") == 0, "Found value \"%s\" at index 1 after removal\n", (char *)list->array[1]);

    value = cm_array_list_get(list, 2);
    test(value == NULL, "Expected no value at index 2 after removal\n");
    cm_array_list_free(list);
}

static void
test_be_to_size_t(void)
{
    print_test_separator_line();
    printf("Testing test_be_to_size_t\n");
    for (size_t i = 0; i < 65536; i++) {
        printf("Testing for %zu\n", i);
        uint8_t *arr = size_t_to_uint8_be(i, 2);
        size_t res = be_to_size_t(arr, 2);
        test(res == i, "Expected value %zu, got %zu\n", i, res);
        free(arr);
    }
    print_test_separator_line();
}

int
main(int argc, char **argv)
{
    test_hash_table_init();
    test_hash_table_put();
    test_cm_array_list_init();
    test_cm_array_list();
    test_cm_array_list_init_size_t();
    test_be_to_size_t();
}
