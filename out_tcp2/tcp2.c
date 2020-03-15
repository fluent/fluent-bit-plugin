/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Fluent Bit
 *  ==========
 *  Copyright (C) 2019      The Fluent Bit Authors
 *  Copyright (C) 2015-2018 Treasure Data Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <fluent-bit/flb_info.h>
#include <fluent-bit/flb_output.h>
#include <fluent-bit/flb_output_plugin.h>
#include <fluent-bit/flb_pack.h>
#include "tcp2.h"

static int cb_tcp_init(struct flb_output_instance *ins,
                          struct flb_config *config, void *data)
{
    (void) data;
    int ret;
    int io_flags = 0;
    const char *tmp;
    struct flb_upstream *upstream;
    struct flb_out_tcp2 *ctx = NULL;

     /* Allocate plugin context */
    ctx = flb_calloc(1, sizeof(struct flb_out_tcp2));
    if(!ctx) {
        return 0;
    }
    ctx->ins = ins;

    /* Set default network configuration if not set */
    flb_output_net_default("127.0.0.1", 5170, ins);

    io_flags = FLB_IO_TCP;

     /* Upstream context */
    upstream = flb_upstream_create(config,
                                   ins->host.name,
                                   ins->host.port,
                                   io_flags, (void *) &ins->tls);
    if (!upstream) {
        flb_plg_error(ctx->ins, "could not create upstream context");
        flb_free(ctx);
        return 0;
    }

    /* Output format */
    ctx->out_format = FLB_PACK_JSON_FORMAT_NONE;
    tmp = flb_output_get_property("format", ins);
    if (tmp) {
        ret = flb_pack_to_json_format_type(tmp);
        if (ret == -1) {
            flb_plg_error(ctx->ins, "unrecognized 'format' option '%s'. "
                          "Using 'msgpack'", tmp);
        }
        else {
            ctx->out_format = ret;
        }
    }

    /* Set different parameters */
    ctx->u = upstream;
    ctx->host = ins->host.name;
    ctx->port = ins->host.port;

    /* Set the plugin context */
    flb_output_set_context(ins, ctx);

    return 0;

}

static void cb_tcp_flush(const void *data, size_t bytes,
                            const char *tag, int tag_len,
                            struct flb_input_instance *i_ins,
                            void *out_context,
                            struct flb_config *config)
{
    int ret = FLB_ERROR;
    size_t bytes_sent;
    flb_sds_t json = NULL;
    flb_sds_t json_date_key = flb_sds_create("date");
    struct flb_upstream *u;
    struct flb_upstream_conn *u_conn;
    struct flb_out_tcp2 *ctx = out_context;

    /* Get upstream context and connection */
    u = ctx->u;
    u_conn = flb_upstream_conn_get(u);
    if (!u_conn) {
        flb_plg_error(ctx->ins, "no upstream connections available to %s:%i",
                      u->tcp_host, u->tcp_port);
        FLB_OUTPUT_RETURN(FLB_RETRY);
    }  


    if (ctx->out_format == FLB_PACK_JSON_FORMAT_NONE) {
        ret = flb_io_net_write(u_conn, data, bytes, &bytes_sent);
    }
    else {
        json = flb_pack_msgpack_to_json_format(data, bytes,
                                               ctx->out_format,
                                               0,
                                               json_date_key);
        if (!json) {
            flb_plg_error(ctx->ins, "error formatting JSON payload");
            flb_upstream_conn_release(u_conn);
            FLB_OUTPUT_RETURN(FLB_ERROR);
        }
        ret = flb_io_net_write(u_conn, json, flb_sds_len(json), &bytes_sent);
        flb_sds_destroy(json);
    }

    if (ret == -1) {
        flb_errno();
        flb_upstream_conn_release(u_conn);
        FLB_OUTPUT_RETURN(FLB_RETRY);
    }

    flb_upstream_conn_release(u_conn);
    FLB_OUTPUT_RETURN(FLB_OK);     
}

static int cb_tcp_exit(void *data, struct flb_config *config)
{
    struct flb_out_tcp2 *ctx = data;

    if (!ctx) {
        return 0;
    }

    if (ctx->u) {
        flb_upstream_destroy(ctx->u);
    }
    flb_free(ctx);
    ctx = NULL;
    
    return 0;
}

struct flb_output_plugin out_tcp2_plugin = {
    .name         = "tcp2",
    .description  = "TCP Output",
    .cb_init      = cb_tcp_init,
    .cb_flush     = cb_tcp_flush,
    .cb_exit      = cb_tcp_exit,
    .flags        = FLB_OUTPUT_NET | FLB_IO_OPT_TLS,
};
