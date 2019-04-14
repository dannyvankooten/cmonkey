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

#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cmonkey_utils.h"

cm_list *
cm_list_init(void)
{
    cm_list *list;
    list = malloc(sizeof(*list));
    if (list == NULL)
        errx(EXIT_FAILURE, "malloc failed");
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    return list;
}

int
cm_list_add(cm_list *list, void *data)
{
    cm_list_node *node = malloc(sizeof(*node));
    if (node == NULL)
        return 0;
    node->data = data;
    node->next = NULL;
    list->length++;
    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
        return 1;
    }

    list->tail->next = node;
    list->tail = node;
    return 1;
}

void
cm_list_free(cm_list *list, void (*free_data) (void *))
{
    if (list == NULL)
        return;
    cm_list_node *list_node = list->head;
    cm_list_node *temp_node = list_node;
    while (list_node != NULL) {
        if (free_data)
            free_data(list_node->data);
        else
            free(list_node->data);
        temp_node = list_node->next;
        free(list_node);
        list_node = temp_node;
    }
    free(list);
}


static
size_t calculate_string_size(long l)
{
    size_t size = 0;
    if (l < 0) {
        size++;
        l = -1 * l;
    }
    while (l >= 10) {
        l /= 10;
        size++;
    }
    return size + 1;
}

char *
long_to_string(long l)
{
    long rem;
    size_t str_size = calculate_string_size(l) + 1;
    char *str = malloc(str_size + 1);
    size_t index = str_size;
    _Bool is_negative = l < 0;
    if (is_negative)
        l = -1 * l;
    str[--index] = 0;
    while (l >= 10) {
        rem = l % 10;
        l /= 10;
        str[--index] = 48 + rem;
    }
    str[--index] = 48 + l;
    if (is_negative)
        str[--index] = '-';
    return str;
}

const char *
bool_to_string(_Bool value)
{
    if (value)
        return "true";
    return "false";
}

cm_hash_table *
cm_hash_table_init(size_t (*hash_func)(void *),
    _Bool (*keycmp) (void *, void *),
    void (*free_key) (void *),
    void (*free_value) (void *))
{
    cm_hash_table *table;
    table = malloc(sizeof(*table));
    if (table == NULL)
        errx(EXIT_FAILURE, "malloc failed");
    table->hash_func = hash_func;
    table->keycmp = keycmp;
    table->free_key = free_key;
    table->free_value = free_value;
    table->table_size = INITIAL_HASHTABLE_SIZE;
    table->table = calloc(table->table_size, sizeof(*table->table));
    if (table->table == NULL)
        errx(EXIT_FAILURE, "malloc failed");
    table->nentries = 0;
    table->nkeys = 0;
    return table;
}

void
cm_hash_table_put(cm_hash_table *hash_table, void *key, void *value)
{
    cm_hash_entry *entry;
    size_t index = hash_table->hash_func(key) % hash_table->table_size;
    cm_list *list_entry = hash_table->table[index];
    if (list_entry == NULL) {
        list_entry = cm_list_init();
        hash_table->table[index] = list_entry;
        hash_table->nentries++;
        //TODO: resize when nentries == table size
    }
    entry = malloc(sizeof*entry);
    if (entry == NULL)
        errx(EXIT_FAILURE, "malloc failed");
    entry->key = key;
    entry->value = value;
    hash_table->nkeys++;
    cm_list_add(list_entry, entry);
}

void *
cm_hash_table_get(cm_hash_table *hash_table, void *key)
{
    size_t index = hash_table->hash_func(key) % hash_table->table_size;
    cm_list *entry_list = hash_table->table[index];
    cm_hash_entry *entry;
    if (entry_list == NULL)
        return NULL;
    cm_list_node *list_node = entry_list->head;
    while (list_node) {
        entry = (cm_hash_entry *) list_node->data;
        if (hash_table->keycmp(entry->key, key))
            return entry->value;
        list_node = list_node->next;
    }
    return NULL;
}

size_t
string_hash_function(void *key)
{
    unsigned long hash = 5381;
    char *str = (char *) key;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

_Bool
string_keycmp(void *key1, void *key2)
{
    char *strkey1 = (char *) key1;
    char *strkey2 = (char *) key2;
    return strcmp(strkey1, strkey2) == 0;
}

static void
free_entry_list(cm_hash_table *table, cm_list *entry_list)
{
    cm_list_node *node = entry_list->head;
    cm_list_node *temp;
    while (node != NULL) {
        cm_hash_entry *entry = (cm_hash_entry *) node->data;
        if (table->free_key != NULL) {
            table->free_key(entry->key);
        }
        if (table->free_value != NULL) {
            table->free_value(entry->value);
        }
        free(entry);
        temp = node->next;
        free(node);
        node = temp;
    }
    free(entry_list);
}

void
cm_hash_table_free(cm_hash_table *table)
{
    for (size_t i = 0; i < table->table_size; i++) {
        cm_list *entry_list = table->table[i];
        if (entry_list != NULL) {
            free_entry_list(table, entry_list);
        }
    }
    free(table->table);
    free(table);
}