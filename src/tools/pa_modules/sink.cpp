/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>
#include <pulse/context.h>
#include <pulse/timeval.h>
#include <pulse/xmalloc.h>
#include <pulse/stream.h>
#include <pulse/mainloop.h>
#include <pulse/introspect.h>
#include <pulse/error.h>

#include <pulsecore/core.h>
#include <pulsecore/core-util.h>
#include <pulsecore/i18n.h>
#include <pulsecore/sink.h>
#include <pulsecore/modargs.h>
#include <pulsecore/log.h>
#include <pulsecore/thread.h>
#include <pulsecore/thread-mq.h>
#include <pulsecore/poll.h>
#include <pulsecore/proplist-util.h>

extern "C"
void pa__done(pa_module *m);

PA_MODULE_AUTHOR("Alexander Couzens");
PA_MODULE_DESCRIPTION("Create a network sink which connects via a stream to a remote PulseAudio server");
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(false);
PA_MODULE_USAGE(
        "server=<address> "
        "sink=<name of the remote sink> "
        "sink_name=<name for the local sink> "
        "sink_properties=<properties for the local sink> "
        "format=<sample format> "
        "channels=<number of channels> "
        "rate=<sample rate> "
        "channel_map=<channel map> "
        "cookie=<cookie file path>"
        );

#define MAX_LATENCY_USEC (200 * PA_USEC_PER_MSEC)
#define TUNNEL_THREAD_FAILED_MAINLOOP 1

struct userdata {
    pa_module *module;
    pa_sink *sink;
    pa_thread *thread;
    pa_thread_mq *thread_mq;
    pa_mainloop *thread_mainloop;
    pa_mainloop_api *thread_mainloop_api;

    pa_context *context;
    pa_stream *stream;

    bool update_stream_bufferattr_after_connect;

    bool connected;

    char *cookie_file;
    char *remote_server;
    char *remote_sink_name;
};

static const char* const valid_modargs[] = {
    "sink_name",
    "sink_properties",
    "server",
    "sink",
    "format",
    "channels",
    "rate",
    "channel_map",
    "cookie",
   /* "reconnect", reconnect if server comes back again - unimplemented */
    NULL,
};

extern "C"
int pa__init(pa_module *m) {
    struct userdata *u = NULL;
    pa_modargs *ma = NULL;
    pa_sink_new_data sink_data;
    pa_sample_spec ss;
    pa_channel_map map;
    const char *remote_server = NULL;
    const char *sink_name = NULL;
    char *default_sink_name = NULL;

    pa_assert(m);

    if (!(ma = pa_modargs_new(m->argument, valid_modargs))) {
        pa_log("Failed to parse module arguments.");
        goto fail;
    }

    ss = m->core->default_sample_spec;
    map = m->core->default_channel_map;
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &ss, &map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        pa_log("Invalid sample format specification or channel map");
        goto fail;
    }

    remote_server = pa_modargs_get_value(ma, "server", NULL);
    if (!remote_server) {
        pa_log("No server given!");
        goto fail;
    }

    u = pa_xnew0(struct userdata, 1);
    u->module = m;
    m->userdata = u;
    u->remote_server = pa_xstrdup(remote_server);
    u->thread_mainloop = pa_mainloop_new();
    if (u->thread_mainloop == NULL) {
        pa_log("Failed to create mainloop");
        goto fail;
    }
    u->thread_mainloop_api = pa_mainloop_get_api(u->thread_mainloop);
    u->cookie_file = pa_xstrdup(pa_modargs_get_value(ma, "cookie", NULL));
    u->remote_sink_name = pa_xstrdup(pa_modargs_get_value(ma, "sink", NULL));

    u->thread_mq = pa_xnew0(pa_thread_mq, 1);
    pa_thread_mq_init_thread_mainloop(u->thread_mq, m->core->mainloop, u->thread_mainloop_api);

    /* Create sink */
    pa_sink_new_data_init(&sink_data);
    sink_data.driver = __FILE__;
    sink_data.module = m;

    default_sink_name = pa_sprintf_malloc("tunnel-sink-new.%s", remote_server);
    sink_name = pa_modargs_get_value(ma, "sink_name", default_sink_name);

    pa_sink_new_data_set_name(&sink_data, sink_name);
    pa_sink_new_data_set_sample_spec(&sink_data, &ss);
    pa_sink_new_data_set_channel_map(&sink_data, &map);

    pa_proplist_sets(sink_data.proplist, PA_PROP_DEVICE_CLASS, "sound");
    pa_proplist_setf(sink_data.proplist,
                     PA_PROP_DEVICE_DESCRIPTION,
                     _("Tunnel to %s/%s"),
                     remote_server,
                     pa_strempty(u->remote_sink_name));

    if (pa_modargs_get_proplist(ma, "sink_properties", sink_data.proplist, PA_UPDATE_REPLACE) < 0) {
        pa_log("Invalid properties");
        pa_sink_new_data_done(&sink_data);
        goto fail;
    }
    if (!(u->sink = pa_sink_new(m->core, &sink_data, pa_sink_flags_t(PA_SINK_LATENCY | PA_SINK_DYNAMIC_LATENCY | PA_SINK_NETWORK)))) {
        pa_log("Failed to create sink.");
        pa_sink_new_data_done(&sink_data);
        goto fail;
    }

    // pa_sink_new_data_done(&sink_data);
    // u->sink->userdata = u;
    // u->sink->parent.process_msg = sink_process_msg_cb;
    // u->sink->update_requested_latency = sink_update_requested_latency_cb;
    // pa_sink_set_latency_range(u->sink, 0, MAX_LATENCY_USEC);

    // /* set thread message queue */
    // pa_sink_set_asyncmsgq(u->sink, u->thread_mq->inq);

    // if (!(u->thread = pa_thread_new("tunnel-sink", thread_func, u))) {
    //     pa_log("Failed to create thread.");
    //     goto fail;
    // }

    // pa_sink_put(u->sink);
    pa_modargs_free(ma);
    pa_xfree(default_sink_name);

    return 0;

fail:
    if (ma)
        pa_modargs_free(ma);

    if (default_sink_name)
        pa_xfree(default_sink_name);

    pa__done(m);

    return -1;
}

extern "C"
void pa__done(pa_module *m) {
    struct userdata *u;

    pa_assert(m);

    if (!(u = (userdata*)m->userdata))
        return;

    if (u->sink)
        pa_sink_unlink(u->sink);

    if (u->thread) {
        pa_asyncmsgq_send(u->thread_mq->inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    if (u->thread_mq) {
        pa_thread_mq_done(u->thread_mq);
        pa_xfree(u->thread_mq);
    }

    if (u->thread_mainloop)
        pa_mainloop_free(u->thread_mainloop);

    if (u->cookie_file)
        pa_xfree(u->cookie_file);

    if (u->remote_sink_name)
        pa_xfree(u->remote_sink_name);

    if (u->remote_server)
        pa_xfree(u->remote_server);

    if (u->sink)
        pa_sink_unref(u->sink);

    pa_xfree(u);
}
