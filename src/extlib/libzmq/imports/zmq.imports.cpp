// Import definitions for libzmq-v143-mt-4_3_6 library
// generated Wed Oct  8 14:57:06 2025 by DylibTool
#ifdef STATIC_LIBLIBZMQ_V143_MT_4_3_6

namespace Dylib { bool libzmq_v143_mt_4_3_6_Loaded() { return true; } }


#else // STATIC_LIBLIBZMQ_V143_MT_4_3_6

#include "zmq.imports.h"
#include "DylibImports.h"
namespace { extern const Dylib::Import* functionImports; }

// Here you may specify a custom error message to be displayed when the library cannot be found.
static const char* notFoundMsg = "";
// Here you may specify an URL to some local or remote help resource.
static const char* notFoundURL = "";
RegisterDylibWithAliases( libzmq_v143_mt_4_3_6, "libzmq-v143-mt-4_3_6", functionImports, notFoundMsg, notFoundURL );

decltype(&__1d1e7041_zmq_atomic_counter_dec) zmq_atomic_counter_dec = 0;
decltype(&__72770433_zmq_atomic_counter_destroy) zmq_atomic_counter_destroy = 0;
decltype(&__161e704c_zmq_atomic_counter_inc) zmq_atomic_counter_inc = 0;
decltype(&__1d0a704b_zmq_atomic_counter_new) zmq_atomic_counter_new = 0;
decltype(&__1d097056_zmq_atomic_counter_set) zmq_atomic_counter_set = 0;
decltype(&__19110536_zmq_atomic_counter_value) zmq_atomic_counter_value = 0;
decltype(&__18041f3b_zmq_bind) zmq_bind = 0;
decltype(&__7c011e2c_zmq_close) zmq_close = 0;
decltype(&__7c616b31_zmq_connect) zmq_connect = 0;
//decltype(&__0c040e1c_zmq_connect_peer) zmq_connect_peer = 0;
decltype(&__0f130374_zmq_ctx_destroy) zmq_ctx_destroy = 0;
decltype(&__7e7c7d00_zmq_ctx_get) zmq_ctx_get = 0;
//decltype(&__1b04095f_zmq_ctx_get_ext) zmq_ctx_get_ext = 0;
decltype(&__777c7e00_zmq_ctx_new) zmq_ctx_new = 0;
decltype(&__6a7c7d00_zmq_ctx_set) zmq_ctx_set = 0;
//decltype(&__0f04095f_zmq_ctx_set_ext) zmq_ctx_set_ext = 0;
decltype(&__0e1e0b1a_zmq_ctx_shutdown) zmq_ctx_shutdown = 0;
decltype(&__6d7c7b6d_zmq_ctx_term) zmq_ctx_term = 0;
decltype(&__77370925_zmq_curve_keypair) zmq_curve_keypair = 0;
decltype(&__1e2b1a3f_zmq_curve_public) zmq_curve_public = 0;
decltype(&__7d6d0736_zmq_device) zmq_device = 0;
decltype(&__121e6c59_zmq_disconnect) zmq_disconnect = 0;
decltype(&__701f0331_zmq_errno) zmq_errno = 0;
decltype(&__021f6e43_zmq_getsockopt) zmq_getsockopt = 0;
decltype(&__120c025f_zmq_has) zmq_has = 0;
decltype(&__1303182b_zmq_init) zmq_init = 0;
//decltype(&__10021831_zmq_join) zmq_join = 0;
//decltype(&__73081029_zmq_leave) zmq_leave = 0;
decltype(&__11727973_zmq_msg_close) zmq_msg_close = 0;
decltype(&__74716679_zmq_msg_copy) zmq_msg_copy = 0;
decltype(&__737f6261_zmq_msg_data) zmq_msg_data = 0;
decltype(&__707b6200_zmq_msg_get) zmq_msg_get = 0;
decltype(&__707b6273_zmq_msg_gets) zmq_msg_gets = 0;
//decltype(&__006c7975_zmq_msg_group) zmq_msg_group = 0;
decltype(&__7e707f74_zmq_msg_init) zmq_msg_init = 0;
//decltype(&__47777812_zmq_msg_init_buffer) zmq_msg_init_buffer = 0;
decltype(&__40141e00_zmq_msg_init_data) zmq_msg_init_data = 0;
decltype(&__4403160e_zmq_msg_init_size) zmq_msg_init_size = 0;
decltype(&__7a716465_zmq_msg_more) zmq_msg_more = 0;
decltype(&__7a716065_zmq_msg_move) zmq_msg_move = 0;
decltype(&__657b7576_zmq_msg_recv) zmq_msg_recv = 0;
//decltype(&__657b042b_zmq_msg_routing_id) zmq_msg_routing_id = 0;
decltype(&__647b7864_zmq_msg_send) zmq_msg_send = 0;
decltype(&__647b6200_zmq_msg_set) zmq_msg_set = 0;
//decltype(&__73090d2a_zmq_msg_set_group) zmq_msg_set_group = 0;
//decltype(&__161e7074_zmq_msg_set_routing_id) zmq_msg_set_routing_id = 0;
decltype(&__64776c65_zmq_msg_size) zmq_msg_size = 0;
decltype(&__0a021d33_zmq_poll) zmq_poll = 0;
//decltype(&__0b144252_zmq_poller_add) zmq_poller_add = 0;
//decltype(&__6f141d34_zmq_poller_add_fd) zmq_poller_add_fd = 0;
//decltype(&__657a3625_zmq_poller_destroy) zmq_poller_destroy = 0;
//decltype(&__0b704255_zmq_poller_fd) zmq_poller_fd = 0;
//decltype(&__79142b38_zmq_poller_modify) zmq_poller_modify = 0;
//decltype(&__794b4d5c_zmq_poller_modify_fd) zmq_poller_modify_fd = 0;
//decltype(&__0a07425d_zmq_poller_new) zmq_poller_new = 0;
//decltype(&__6f1d2d37_zmq_poller_remove) zmq_poller_remove = 0;
//decltype(&__6f424b53_zmq_poller_remove_fd) zmq_poller_remove_fd = 0;
//decltype(&__060a2740_zmq_poller_size) zmq_poller_size = 0;
//decltype(&__0e193644_zmq_poller_wait) zmq_poller_wait = 0;
//decltype(&__6f755a1b_zmq_poller_wait_all) zmq_poller_wait_all = 0;
//decltype(&__661d1e33_zmq_ppoll) zmq_ppoll = 0;
decltype(&__731f1e27_zmq_proxy) zmq_proxy = 0;
decltype(&__74497a32_zmq_proxy_steerable) zmq_proxy_steerable = 0;
decltype(&__08081229_zmq_recv) zmq_recv = 0;
decltype(&__61676429_zmq_recviov) zmq_recviov = 0;
decltype(&__657b7529_zmq_recvmsg) zmq_recvmsg = 0;
decltype(&__09081f3b_zmq_send) zmq_send = 0;
decltype(&__251f7055_zmq_send_const) zmq_send_const = 0;
decltype(&__6067693b_zmq_sendiov) zmq_sendiov = 0;
decltype(&__647b783b_zmq_sendmsg) zmq_sendmsg = 0;
decltype(&__161f6e43_zmq_setsockopt) zmq_setsockopt = 0;
decltype(&__7901143a_zmq_sleep) zmq_sleep = 0;
decltype(&__6c761234_zmq_socket) zmq_socket = 0;
//decltype(&__7a130108_zmq_socket_get_peer_state) zmq_socket_get_peer_state = 0;
decltype(&__6c6a242d_zmq_socket_monitor) zmq_socket_monitor = 0;
//decltype(&__2e1a6a4f_zmq_socket_monitor_pipes_stats) zmq_socket_monitor_pipes_stats = 0;
//decltype(&__66766d56_zmq_socket_monitor_versioned) zmq_socket_monitor_versioned = 0;
decltype(&__7343182e_zmq_stopwatch_intermediate) zmq_stopwatch_intermediate = 0;
decltype(&__77556d38_zmq_stopwatch_start) zmq_stopwatch_start = 0;
decltype(&__79571938_zmq_stopwatch_stop) zmq_stopwatch_stop = 0;
decltype(&__7b6b6c48_zmq_strerror) zmq_strerror = 0;
decltype(&__0e080332_zmq_term) zmq_term = 0;
decltype(&__00120556_zmq_threadclose) zmq_threadclose = 0;
decltype(&__0e13044e_zmq_threadstart) zmq_threadstart = 0;
decltype(&__1813435b_zmq_timers_add) zmq_timers_add = 0;
decltype(&__7119203c_zmq_timers_cancel) zmq_timers_cancel = 0;
decltype(&__767d372c_zmq_timers_destroy) zmq_timers_destroy = 0;
decltype(&__7077202a_zmq_timers_execute) zmq_timers_execute = 0;
decltype(&__19004354_zmq_timers_new) zmq_timers_new = 0;
decltype(&__1904263c_zmq_timers_reset) zmq_timers_reset = 0;
decltype(&__01161552_zmq_timers_set_interval) zmq_timers_set_interval = 0;
decltype(&__606e2621_zmq_timers_timeout) zmq_timers_timeout = 0;
decltype(&__61671336_zmq_unbind) zmq_unbind = 0;
decltype(&__65676d2c_zmq_version) zmq_version = 0;
decltype(&__0055276f_zmq_z85_decode) zmq_z85_decode = 0;
decltype(&__015e276f_zmq_z85_encode) zmq_z85_encode = 0;


namespace {
const Dylib::Import functionImports_[] =
{
  { "zmq_atomic_counter_dec", (void**)&zmq_atomic_counter_dec, Dylib::Import::cMangled },
  { "zmq_atomic_counter_destroy", (void**)&zmq_atomic_counter_destroy, Dylib::Import::cMangled },
  { "zmq_atomic_counter_inc", (void**)&zmq_atomic_counter_inc, Dylib::Import::cMangled },
  { "zmq_atomic_counter_new", (void**)&zmq_atomic_counter_new, Dylib::Import::cMangled },
  { "zmq_atomic_counter_set", (void**)&zmq_atomic_counter_set, Dylib::Import::cMangled },
  { "zmq_atomic_counter_value", (void**)&zmq_atomic_counter_value, Dylib::Import::cMangled },
  { "zmq_bind", (void**)&zmq_bind, Dylib::Import::cMangled },
  { "zmq_close", (void**)&zmq_close, Dylib::Import::cMangled },
  { "zmq_connect", (void**)&zmq_connect, Dylib::Import::cMangled },
//  { "zmq_connect_peer", (void**)&zmq_connect_peer, Dylib::Import::cMangled },
  { "zmq_ctx_destroy", (void**)&zmq_ctx_destroy, Dylib::Import::cMangled },
  { "zmq_ctx_get", (void**)&zmq_ctx_get, Dylib::Import::cMangled },
//  { "zmq_ctx_get_ext", (void**)&zmq_ctx_get_ext, Dylib::Import::cMangled },
  { "zmq_ctx_new", (void**)&zmq_ctx_new, Dylib::Import::cMangled },
  { "zmq_ctx_set", (void**)&zmq_ctx_set, Dylib::Import::cMangled },
//  { "zmq_ctx_set_ext", (void**)&zmq_ctx_set_ext, Dylib::Import::cMangled },
  { "zmq_ctx_shutdown", (void**)&zmq_ctx_shutdown, Dylib::Import::cMangled },
  { "zmq_ctx_term", (void**)&zmq_ctx_term, Dylib::Import::cMangled },
  { "zmq_curve_keypair", (void**)&zmq_curve_keypair, Dylib::Import::cMangled },
  { "zmq_curve_public", (void**)&zmq_curve_public, Dylib::Import::cMangled },
  { "zmq_device", (void**)&zmq_device, Dylib::Import::cMangled },
  { "zmq_disconnect", (void**)&zmq_disconnect, Dylib::Import::cMangled },
  { "zmq_errno", (void**)&zmq_errno, Dylib::Import::cMangled },
  { "zmq_getsockopt", (void**)&zmq_getsockopt, Dylib::Import::cMangled },
  { "zmq_has", (void**)&zmq_has, Dylib::Import::cMangled },
  { "zmq_init", (void**)&zmq_init, Dylib::Import::cMangled },
//  { "zmq_join", (void**)&zmq_join, Dylib::Import::cMangled },
//  { "zmq_leave", (void**)&zmq_leave, Dylib::Import::cMangled },
  { "zmq_msg_close", (void**)&zmq_msg_close, Dylib::Import::cMangled },
  { "zmq_msg_copy", (void**)&zmq_msg_copy, Dylib::Import::cMangled },
  { "zmq_msg_data", (void**)&zmq_msg_data, Dylib::Import::cMangled },
  { "zmq_msg_get", (void**)&zmq_msg_get, Dylib::Import::cMangled },
  { "zmq_msg_gets", (void**)&zmq_msg_gets, Dylib::Import::cMangled },
//  { "zmq_msg_group", (void**)&zmq_msg_group, Dylib::Import::cMangled },
  { "zmq_msg_init", (void**)&zmq_msg_init, Dylib::Import::cMangled },
//  { "zmq_msg_init_buffer", (void**)&zmq_msg_init_buffer, Dylib::Import::cMangled },
  { "zmq_msg_init_data", (void**)&zmq_msg_init_data, Dylib::Import::cMangled },
  { "zmq_msg_init_size", (void**)&zmq_msg_init_size, Dylib::Import::cMangled },
  { "zmq_msg_more", (void**)&zmq_msg_more, Dylib::Import::cMangled },
  { "zmq_msg_move", (void**)&zmq_msg_move, Dylib::Import::cMangled },
  { "zmq_msg_recv", (void**)&zmq_msg_recv, Dylib::Import::cMangled },
//  { "zmq_msg_routing_id", (void**)&zmq_msg_routing_id, Dylib::Import::cMangled },
  { "zmq_msg_send", (void**)&zmq_msg_send, Dylib::Import::cMangled },
  { "zmq_msg_set", (void**)&zmq_msg_set, Dylib::Import::cMangled },
//  { "zmq_msg_set_group", (void**)&zmq_msg_set_group, Dylib::Import::cMangled },
//  { "zmq_msg_set_routing_id", (void**)&zmq_msg_set_routing_id, Dylib::Import::cMangled },
  { "zmq_msg_size", (void**)&zmq_msg_size, Dylib::Import::cMangled },
  { "zmq_poll", (void**)&zmq_poll, Dylib::Import::cMangled },
//  { "zmq_poller_add", (void**)&zmq_poller_add, Dylib::Import::cMangled },
//  { "zmq_poller_add_fd", (void**)&zmq_poller_add_fd, Dylib::Import::cMangled },
//  { "zmq_poller_destroy", (void**)&zmq_poller_destroy, Dylib::Import::cMangled },
//  { "zmq_poller_fd", (void**)&zmq_poller_fd, Dylib::Import::cMangled },
//  { "zmq_poller_modify", (void**)&zmq_poller_modify, Dylib::Import::cMangled },
//  { "zmq_poller_modify_fd", (void**)&zmq_poller_modify_fd, Dylib::Import::cMangled },
//  { "zmq_poller_new", (void**)&zmq_poller_new, Dylib::Import::cMangled },
//  { "zmq_poller_remove", (void**)&zmq_poller_remove, Dylib::Import::cMangled },
//  { "zmq_poller_remove_fd", (void**)&zmq_poller_remove_fd, Dylib::Import::cMangled },
//  { "zmq_poller_size", (void**)&zmq_poller_size, Dylib::Import::cMangled },
//  { "zmq_poller_wait", (void**)&zmq_poller_wait, Dylib::Import::cMangled },
//  { "zmq_poller_wait_all", (void**)&zmq_poller_wait_all, Dylib::Import::cMangled },
//  { "zmq_ppoll", (void**)&zmq_ppoll, Dylib::Import::cMangled },
  { "zmq_proxy", (void**)&zmq_proxy, Dylib::Import::cMangled },
  { "zmq_proxy_steerable", (void**)&zmq_proxy_steerable, Dylib::Import::cMangled },
  { "zmq_recv", (void**)&zmq_recv, Dylib::Import::cMangled },
  { "zmq_recviov", (void**)&zmq_recviov, Dylib::Import::cMangled },
  { "zmq_recvmsg", (void**)&zmq_recvmsg, Dylib::Import::cMangled },
  { "zmq_send", (void**)&zmq_send, Dylib::Import::cMangled },
  { "zmq_send_const", (void**)&zmq_send_const, Dylib::Import::cMangled },
  { "zmq_sendiov", (void**)&zmq_sendiov, Dylib::Import::cMangled },
  { "zmq_sendmsg", (void**)&zmq_sendmsg, Dylib::Import::cMangled },
  { "zmq_setsockopt", (void**)&zmq_setsockopt, Dylib::Import::cMangled },
  { "zmq_sleep", (void**)&zmq_sleep, Dylib::Import::cMangled },
  { "zmq_socket", (void**)&zmq_socket, Dylib::Import::cMangled },
//  { "zmq_socket_get_peer_state", (void**)&zmq_socket_get_peer_state, Dylib::Import::cMangled },
  { "zmq_socket_monitor", (void**)&zmq_socket_monitor, Dylib::Import::cMangled },
//  { "zmq_socket_monitor_pipes_stats", (void**)&zmq_socket_monitor_pipes_stats, Dylib::Import::cMangled },
//  { "zmq_socket_monitor_versioned", (void**)&zmq_socket_monitor_versioned, Dylib::Import::cMangled },
  { "zmq_stopwatch_intermediate", (void**)&zmq_stopwatch_intermediate, Dylib::Import::cMangled },
  { "zmq_stopwatch_start", (void**)&zmq_stopwatch_start, Dylib::Import::cMangled },
  { "zmq_stopwatch_stop", (void**)&zmq_stopwatch_stop, Dylib::Import::cMangled },
  { "zmq_strerror", (void**)&zmq_strerror, Dylib::Import::cMangled },
  { "zmq_term", (void**)&zmq_term, Dylib::Import::cMangled },
  { "zmq_threadclose", (void**)&zmq_threadclose, Dylib::Import::cMangled },
  { "zmq_threadstart", (void**)&zmq_threadstart, Dylib::Import::cMangled },
  { "zmq_timers_add", (void**)&zmq_timers_add, Dylib::Import::cMangled },
  { "zmq_timers_cancel", (void**)&zmq_timers_cancel, Dylib::Import::cMangled },
  { "zmq_timers_destroy", (void**)&zmq_timers_destroy, Dylib::Import::cMangled },
  { "zmq_timers_execute", (void**)&zmq_timers_execute, Dylib::Import::cMangled },
  { "zmq_timers_new", (void**)&zmq_timers_new, Dylib::Import::cMangled },
  { "zmq_timers_reset", (void**)&zmq_timers_reset, Dylib::Import::cMangled },
  { "zmq_timers_set_interval", (void**)&zmq_timers_set_interval, Dylib::Import::cMangled },
  { "zmq_timers_timeout", (void**)&zmq_timers_timeout, Dylib::Import::cMangled },
  { "zmq_unbind", (void**)&zmq_unbind, Dylib::Import::cMangled },
  { "zmq_version", (void**)&zmq_version, Dylib::Import::cMangled },
  { "zmq_z85_decode", (void**)&zmq_z85_decode, Dylib::Import::cMangled },
  { "zmq_z85_encode", (void**)&zmq_z85_encode, Dylib::Import::cMangled },
  { 0, 0, 0 }
};
const Dylib::Import* functionImports = functionImports_;
}

#endif // STATIC_LIBLIBZMQ_V143_MT_4_3_6

