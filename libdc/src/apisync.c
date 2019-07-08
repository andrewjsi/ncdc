#include <dc/apisync.h>
#include <dc/refable.h>

#include "internal.h"

struct dc_api_sync_
{
    dc_refable_t ref;

    int code;

    char *buffer;
    size_t bufferlen;
    FILE *stream;

    pthread_mutex_t mtx;
    pthread_cond_t cnd;

    CURL *easy;
    CURLM *curl;
    struct curl_slist *list;
};

static void dc_api_sync_free(dc_api_sync_t s)
{
    return_if_true(s == NULL,);

    pthread_mutex_lock(&s->mtx);

    pthread_cond_destroy(&s->cnd);
    pthread_mutex_destroy(&s->mtx);

    if (s->list != NULL) {
        curl_slist_free_all(s->list);
        s->list = NULL;
    }

    if (s->easy != NULL) {
        if (s->curl != NULL) {
            curl_multi_remove_handle(s->curl, s->easy);
        }
        curl_easy_cleanup(s->easy);
        s->easy = NULL;
    }

    if (s->stream != NULL) {
        fclose(s->stream);
        s->stream = NULL;
    }

    free(s->buffer);
    s->buffer = NULL;
    s->bufferlen = 0;

    free(s);
}

dc_api_sync_t dc_api_sync_new(CURLM *curl, CURL *easy)
{
    dc_api_sync_t ptr = calloc(1, sizeof(struct dc_api_sync_));
    return_if_true(ptr == NULL, NULL);

    ptr->easy = easy;
    ptr->ref.cleanup = (dc_cleanup_t)dc_api_sync_free;

    ptr->stream = open_memstream(&ptr->buffer, &ptr->bufferlen);
    if (ptr->stream == NULL) {
        free(ptr);
        return NULL;
    }

    pthread_mutex_init(&ptr->mtx, NULL);
    pthread_cond_init(&ptr->cnd, NULL);

    ptr->list = curl_slist_append(NULL, "");

    return dc_ref(ptr);
}

struct curl_slist *dc_api_sync_list(dc_api_sync_t sync)
{
    return_if_true(sync == NULL, NULL);
    return sync->list;
}

FILE *dc_api_sync_stream(dc_api_sync_t sync)
{
    return_if_true(sync == NULL, NULL);
    return sync->stream;
}

char const *dc_api_sync_data(dc_api_sync_t sync)
{
    return_if_true(sync == NULL, NULL);
    return sync->buffer;
}

size_t dc_api_sync_datalen(dc_api_sync_t sync)
{
    return_if_true(sync == NULL, 0L);
    return sync->bufferlen;
}

int dc_api_sync_code(dc_api_sync_t sync)
{
    return_if_true(sync == NULL, 0L);
    return sync->code;
}

bool dc_api_sync_wait(dc_api_sync_t sync)
{
    return_if_true(sync == NULL, false);

    if (sync->stream == NULL && sync->buffer != NULL) {
        return true;
    }

    pthread_mutex_lock(&sync->mtx);
    pthread_cond_wait(&sync->cnd, &sync->mtx);
    pthread_mutex_unlock(&sync->mtx);

    return (sync->stream == NULL && sync->buffer != NULL);
}

void dc_api_sync_finish(dc_api_sync_t sync, int code)
{
    return_if_true(sync == NULL,);

    pthread_mutex_lock(&sync->mtx);
    sync->code = code;
    if (sync->stream != NULL) {
        fclose(sync->stream);
        sync->stream = NULL;
    }
    pthread_cond_broadcast(&sync->cnd);
    pthread_mutex_unlock(&sync->mtx);
}
