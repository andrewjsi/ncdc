/*
 * Part of ncdc - a discord client for the console
 * Copyright (C) 2019 Florian Stinglmayr <fstinglmayr@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <dc/channel.h>

#include "internal.h"

struct dc_channel_
{
    dc_refable_t ref;

    dc_channel_type_t type;

    /* snowflake of the channel
     */
    char *id;

    /* Guild ID this channel belongs to, may be NULL
     */
    char *guild_id;

    /* Name of the channel
     */
    char *name;

    /* Slaneeshi filth?
     */
    bool nsfw;

    /* ID of last message in the channel
     */
    char *last_message_id;

    /* list of recipients, array of dc_account_t
     */
    GPtrArray *recipients;

    /* Snowflake of the owner
     */
    char *owner_id;

    /* ID of the parent channel or bot
     */
    char *parent_id;

    /*  application ID of the group DM creator if it is bot-created
     */
    char *application_id;

    GHashTable *messages_byid;
    GPtrArray *messages;
    bool new_messages;
};

static void dc_channel_free(dc_channel_t c)
{
    return_if_true(c == NULL,);

    free(c->id);
    free(c->guild_id);
    free(c->name);
    free(c->last_message_id);
    free(c->owner_id);
    free(c->parent_id);
    free(c->application_id);

    if (c->recipients != NULL) {
        g_ptr_array_unref(c->recipients);
        c->recipients = NULL;
    }

    if (c->messages != NULL) {
        g_ptr_array_unref(c->messages);
        c->messages = NULL;
    }

    if (c->messages_byid != NULL) {
        g_hash_table_unref(c->messages_byid);
        c->messages_byid = NULL;
    }

    free(c);
}

dc_channel_t dc_channel_new(void)
{
    dc_channel_t c = calloc(1, sizeof(struct dc_channel_));
    return_if_true(c == NULL, NULL);

    c->ref.cleanup = (dc_cleanup_t)dc_channel_free;

    c->recipients = g_ptr_array_new_with_free_func(
        (GDestroyNotify)dc_unref
        );

    c->messages = g_ptr_array_new_with_free_func(
        (GDestroyNotify)dc_unref
        );

    c->messages_byid = g_hash_table_new_full(
        g_str_hash, g_str_equal,
        free, dc_unref
        );

    return dc_ref(c);
}

dc_channel_t dc_channel_from_json(json_t *j)
{
    json_t *v = NULL;

    dc_channel_t c = dc_channel_new();

    goto_if_true(!json_is_object(j), error);

    v = json_object_get(j, "id");
    goto_if_true(v == NULL || !json_is_string(v), error);
    c->id = strdup(json_string_value(v));

    v = json_object_get(j, "type");
    goto_if_true(v == NULL || !json_is_integer(v), error);
    c->type = json_integer_value(v);

    v = json_object_get(j, "guild_id");
    if (v != NULL && json_is_string(v)) {
        c->guild_id = strdup(json_string_value(v));
    }

    v = json_object_get(j, "name");
    if (v != NULL && json_is_string(v)) {
        c->name = strdup(json_string_value(v));
    }

    v = json_object_get(j, "nsfw");
    if (v != NULL && json_is_boolean(v)) {
        c->nsfw = json_boolean_value(v);
    }

    v = json_object_get(j, "last_message_id");
    if (v != NULL && json_is_string(v)) {
        c->last_message_id = strdup(json_string_value(v));
    }

    v = json_object_get(j, "owner_id");
    if (v != NULL && json_is_string(v)) {
        c->owner_id = strdup(json_string_value(v));
    }

    v = json_object_get(j, "parent_id");
    if (v != NULL && json_is_string(v)) {
        c->parent_id = strdup(json_string_value(v));
    }

    v = json_object_get(j, "application_id");
    if (v != NULL && json_is_string(v)) {
        c->application_id = strdup(json_string_value(v));
    }

    v = json_object_get(j, "recipients");
    if (v != NULL && json_is_array(v)) {
        json_t *i = NULL;
        size_t idx = 0;

        json_array_foreach(v, idx, i) {
            dc_account_t a = dc_account_from_json(i);
            if (a != NULL) {
                g_ptr_array_add(c->recipients, a);
            }
        }
    }

    return c;

error:

    dc_unref(c);
    return NULL;
}

json_t *dc_channel_to_json(dc_channel_t c)
{
    json_t *j = NULL;

    return_if_true(c->id == NULL, NULL);

    j = json_object();
    return_if_true(j == NULL, NULL);

    /* I was so close in making a J_SET() macro for my lazy ass.
     */
    json_object_set_new(j, "id", json_string(c->id));
    json_object_set_new(j, "type", json_integer(c->type));

    /* TODO: tribool to see if it was actually set, or assume "false"
     *       is a sane default and continue on.
     */
    json_object_set_new(j, "nsfw", json_boolean(c->nsfw));

    if (c->guild_id != NULL) {
        json_object_set_new(j, "guild_id", json_string(c->guild_id));
    }

    if (c->name != NULL) {
        json_object_set_new(j, "name", json_string(c->name));
    }

    if (c->last_message_id != NULL) {
        json_object_set_new(j, "last_message_id",
                            json_string(c->last_message_id));
    }

    if (c->owner_id != NULL) {
        json_object_set_new(j, "owner_id", json_string(c->owner_id));
    }

    if (c->parent_id != NULL) {
        json_object_set_new(j, "parent_id", json_string(c->parent_id));
    }

    if (c->application_id != NULL) {
        json_object_set_new(j, "application_id",
                            json_string(c->application_id));
    }

    if (c->recipients != NULL && c->recipients->len > 0) {
        size_t i = 0;
        json_t *arr = json_array();

        for (i = 0; i < c->recipients->len; i++) {
            dc_account_t acc = g_ptr_array_index(c->recipients, i);
            json_t *a = dc_account_to_json(acc);
            if (a != NULL) {
                json_array_append_new(arr, a);
            }
        }

        json_object_set_new(j, "recipients", arr);
    }

    return j;
}

char const *dc_channel_name(dc_channel_t c)
{
    return_if_true(c == NULL, NULL);
    return c->name;
}

char const *dc_channel_id(dc_channel_t c)
{
    return_if_true(c == NULL, NULL);
    return c->id;
}

char const *dc_channel_parent_id(dc_channel_t c)
{
    return_if_true(c == NULL, NULL);
    return c->parent_id;
}

dc_channel_type_t dc_channel_type(dc_channel_t c)
{
    return_if_true(c == NULL, -1);
    return c->type;
}

bool dc_channel_is_dm(dc_channel_t c)
{
    return_if_true(c == NULL, false);
    return (c->type == CHANNEL_TYPE_GROUP_DM ||
            c->type == CHANNEL_TYPE_DM
        );
}

void dc_channel_set_type(dc_channel_t c, dc_channel_type_t t)
{
    return_if_true(c == NULL,);
    c->type = t;
}

size_t dc_channel_recipients(dc_channel_t c)
{
    return_if_true(c == NULL || c->recipients == NULL, 0);
    return c->recipients->len;
}

dc_account_t dc_channel_nth_recipient(dc_channel_t c, size_t i)
{
    return_if_true(c == NULL || c->recipients == NULL, NULL);
    return_if_true(i >= c->recipients->len, NULL);
    return g_ptr_array_index(c->recipients, i);
}

void dc_channel_add_recipient(dc_channel_t c, dc_account_t a)
{
    return_if_true(c == NULL || a == NULL,);
    g_ptr_array_add(c->recipients, dc_ref(a));
}

bool dc_channel_has_recipient(dc_channel_t c, dc_account_t a)
{
    size_t i = 0;
    return_if_true(c == NULL || a == NULL, false);

    for (i = 0; i < c->recipients->len; i++) {
        if (dc_account_equal(g_ptr_array_index(c->recipients, i), a)) {
            return true;
        }
    }

    return false;
}


size_t dc_channel_messages(dc_channel_t c)
{
    return_if_true(c == NULL || c->messages == NULL, 0);
    return c->messages->len;
}

dc_message_t dc_channel_nth_message(dc_channel_t c, size_t i)
{
    return_if_true(c == NULL || c->messages == NULL, NULL);
    return_if_true(i >= c->messages->len, NULL);
    return g_ptr_array_index(c->messages, i);
}

void dc_channel_add_messages(dc_channel_t c, dc_message_t *m, size_t s)
{
    return_if_true(c == NULL || c->messages == NULL,);
    return_if_true(m == NULL || s == 0,);

    size_t i = 0;

    for (i = 0; i < s; i++) {
        char const *id = dc_message_id(m[i]);
        if (g_hash_table_contains(c->messages_byid, id)) {
            continue;
        }

        g_hash_table_insert(c->messages_byid, strdup(id), dc_ref(m[i]));
        g_ptr_array_add(c->messages, dc_ref(m[i]));

        c->new_messages = true;
    }

    g_ptr_array_sort(c->messages, (GCompareFunc)dc_message_compare);
}

bool dc_channel_compare(dc_channel_t a, dc_channel_t b)
{
    return_if_true(a == NULL || b == NULL, false);
    return_if_true(a->id == NULL || b->id == NULL, false);
    return (strcmp(a->id, b->id) == 0);
}

bool dc_channel_has_new_messages(dc_channel_t c)
{
    return_if_true(c == NULL, false);
    return c->new_messages;
}

void dc_channel_mark_read(dc_channel_t c)
{
    return_if_true(c == NULL,);
    c->new_messages = false;
}
