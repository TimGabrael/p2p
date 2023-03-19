#ifndef P2P_H
#define P2P_H

#include <agent.h>
#include <chrono>
#include <gio/gnetworking.h>
#include <iostream>
#include <thread>



void p2p_init();
// p2p_create_socket("stun.l.google.com", 19302, false);
struct p2p_socket* p2p_create_socket(const char* stun_addr, guint stun_port, bool is_controlling);
void p2p_destroy_socket(struct p2p_socket* sock);
bool p2p_connect(struct p2p_socket* sock, const char* remote_sdp);
void p2p_send_raw(struct p2p_socket* sock, const void* data, unsigned int size);


struct p2p_socket {
    NiceAgent* agent;
    GThread* thread;
    char* sdp;
    GMainLoop* loop;
    guint stream_id;
    bool in_gathering, in_negotiation;
    bool has_gathered, is_connected;
    gboolean is_controlling;
};

#ifdef P2P_IMPLEMENTATION
static void cb_candidate_gathering_done(NiceAgent *agent, guint stream_id, gpointer data) {
    struct p2p_socket* p2p = (struct p2p_socket*)data;
    gchar* sdp = nice_agent_generate_local_sdp(agent);
    p2p->sdp = g_base64_encode((const guchar*)sdp, strnlen(sdp, 10000));
    g_free(sdp);
    p2p->has_gathered = true;
    p2p->in_gathering = false;
}
static void cb_component_state_changed(NiceAgent *agent, guint stream_id, guint component_id, guint state, gpointer data) {
    struct p2p_socket* p2p = (struct p2p_socket*)data;
    if (state == NICE_COMPONENT_STATE_READY) {
        p2p->is_connected = true;
        p2p->in_negotiation = false;
    } else if (state == NICE_COMPONENT_STATE_FAILED) {
        g_main_loop_quit (p2p->loop);
    }
}
static void cb_nice_recv(NiceAgent *agent, guint stream_id, guint component_id, guint len, gchar *buf, gpointer data) {
    //if (len == 1 && buf[0] == '\0')
    //    g_main_loop_quit (gloop);

    printf("received data %d\n", len);

}

static void* p2p__thread_func(void* data) {
    struct p2p_socket* sock = (struct p2p_socket*)data;
    g_main_loop_run(sock->loop);
    return 0;
}
void p2p_init() {
    g_networking_init();
}
struct p2p_socket* p2p_create_socket(const char* stun_addr, guint stun_port, bool is_controlling) {

    struct p2p_socket* out = (struct p2p_socket*)malloc(sizeof(p2p_socket));
    memset(out, 0, sizeof(p2p_socket));

    out->loop = g_main_loop_new(NULL, FALSE);
    out->is_controlling = is_controlling;
    out->agent = nice_agent_new(g_main_loop_get_context(out->loop), NICE_COMPATIBILITY_RFC5245);
    if(out->agent == NULL)  {
        g_error("Failed to create agent!");
    }

    g_object_set(out->agent, "stun-server", stun_addr, NULL);
    g_object_set(out->agent, "stun-server-port", stun_port, NULL);

    g_object_set(out->agent, "controlling-mode", out->is_controlling, NULL);

    g_signal_connect(out->agent, "candidate-gathering-done", G_CALLBACK(cb_candidate_gathering_done), out);
    g_signal_connect(out->agent, "component-state-changed", G_CALLBACK(cb_component_state_changed), out);


    out->stream_id = nice_agent_add_stream(out->agent, 1);
    if(out->stream_id == 0) {
        g_error("Failed to add stream");
    }
    nice_agent_attach_recv(out->agent, out->stream_id, 1, g_main_loop_get_context(out->loop), cb_nice_recv, NULL);

    if(!nice_agent_gather_candidates(out->agent, out->stream_id)) {
        g_error("Failed to start candidate gathering");
    }
    while(!out->has_gathered) {
        std::this_thread::sleep_for(std::chrono::duration<float>(1.0f));
    }

    out->thread = g_thread_new("net_thread", &p2p__thread_func, out);

    return out;
}
void p2p_destroy_socket(struct p2p_socket* sock) {
    g_main_loop_quit (sock->loop);
    g_thread_join(sock->thread);
    g_main_loop_unref(sock->loop);
    g_object_unref(sock->agent);
    if(sock->sdp) g_free(sock->sdp);
    memset(sock, 0, sizeof(struct p2p_socket));
    free(sock);
}
bool p2p_connect(struct p2p_socket* sock, const char* remote_sdp) {
    if(sock->in_negotiation || sock->is_connected) return false;
    sock->in_negotiation = true;
    gsize sdp_len = 0;
    gchar* sdp = (gchar*)g_base64_decode(remote_sdp, &sdp_len);
    bool succeeded = false;
    if(sdp && nice_agent_parse_remote_sdp(sock->agent, sdp) > 0) {
        succeeded = true;
    }
    else {
        sock->in_negotiation = false;
    }
    g_free(sdp);

    return succeeded;
}
void p2p_send_raw(struct p2p_socket* sock, const void* data, unsigned int size) {
    if(!sock->is_connected) return;

    nice_agent_send(sock->agent, sock->stream_id, 1, size, (const gchar*)data);
}

#endif // P2P_IMPLEMENTATION
#endif // P2P_H
