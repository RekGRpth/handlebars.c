 /**
 * Copyright (C) 2020 John Boehr
 *
 * This file is part of handlebars.c.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>
#include <talloc.h>

#define HANDLEBARS_MAP_PRIVATE

#include "handlebars.h"
#include "handlebars_map.h"
#include "handlebars_memory.h"
#include "handlebars_value.h"
#include "utils.h"



char mkchar(unsigned long i) {
    return (char) (32 + (i % (126 - 32)));
}

START_TEST(test_map)
{
#define STRSIZE 128
    size_t i;
    size_t pos = 0;
    size_t count = 10000;
    struct handlebars_map * map = handlebars_map_ctor(context, 0);
    struct handlebars_value * value;
    struct handlebars_map_entry * entry;
    struct handlebars_map_entry * tmp_entry;
    struct handlebars_string ** strings = handlebars_talloc_array(context, struct handlebars_string *, count);
    struct handlebars_string * key;

    // Seed so it's determinisitic
    srand(0x5d0);

    // Generate a bunch of random strings
    for( i = 0; i < count; i++ ) {
        char tmp[STRSIZE];
        size_t l = (rand() % (STRSIZE - 4)) + 4;
        size_t j;
        for( j = 0; j < l; j++ ) {
            tmp[j] = mkchar(rand());
        }
        tmp[j] = 0;
        strings[i] = handlebars_string_ctor(context, tmp, j - 1);
    }

    // Add them all to the map
    for( i = 0; i < count; i++ ) {
        key = strings[i];

        // There can be duplicate strings - skip those
        if (handlebars_map_find(map, key)) {
            continue;
        }

        value = talloc_steal(map, handlebars_value_ctor(context));
        handlebars_value_integer(value, pos++);
        handlebars_map_add(map, key, value);
    }

    fprintf(
        stderr,
        "ENTRIES: %ld, "
        "TABLE SIZE: %ld, "
        "COLLISIONS: %ld, "
        "LOADFACTOR: %d\n",
        map->i,
        map->table_capacity,
        map->collisions,
        handlebars_map_load_factor(map)
    );

    // Make sure we can iterate over the map in insertion order
    pos = 0;
    handlebars_map_foreach(map, entry, tmp_entry) {
        ck_assert_uint_eq(pos++, handlebars_value_get_intval(entry->value));
    } handlebars_map_foreach_end();

    // Remove everything
    i = 0;
    do {
        struct handlebars_string *key = handlebars_map_get_key_at_index(map, 0);
        struct handlebars_string *key2 = handlebars_map_get_key_at_index(map, 1);
        if (!key) break;
        struct handlebars_value * value = handlebars_map_find(map, key);
        ck_assert_ptr_ne(NULL, value);
        handlebars_map_remove(map, key);

        // make sure the count of items in the map is accurate
        ck_assert_uint_eq(--pos, handlebars_map_count(map));

        // make sure the right element was removed
        ck_assert_int_eq(i++, handlebars_value_get_intval(value));

        if (key2) {
            value = handlebars_map_find(map, key2);
            ck_assert_ptr_ne(NULL, value);
            ck_assert_int_eq(i, handlebars_value_get_intval(value));
        }
    } while(1);

    // Make sure it's empty
    ck_assert_uint_eq(handlebars_map_count(map), 0);
    ck_assert_ptr_eq(handlebars_map_get_key_at_index(map, 0), NULL);

    // Free
    handlebars_map_dtor(map);
}
END_TEST

Suite * parser_suite(void)
{
    Suite * s = suite_create("Map");

    REGISTER_TEST_FIXTURE(s, test_map, "Map");

    return s;
}

int main(void)
{
    int number_failed;
    int memdebug;
    int error;

    talloc_set_log_stderr();

    // Check if memdebug enabled
    memdebug = getenv("MEMDEBUG") ? atoi(getenv("MEMDEBUG")) : 0;
    if( memdebug ) {
        talloc_enable_leak_report_full();
    }

    // Set up test suite
    Suite * s = parser_suite();
    SRunner * sr = srunner_create(s);
    if( IS_WIN || memdebug ) {
        srunner_set_fork_status(sr, CK_NOFORK);
    }
    srunner_run_all(sr, CK_ENV);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    error = (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

    // Generate report for memdebug
    if( memdebug ) {
        talloc_report_full(NULL, stderr);
    }

    // Return
    return error;
}
