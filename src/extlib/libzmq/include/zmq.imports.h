// Import definitions for libzmq-v143-mt-4_3_6 library
// generated Wed Oct  8 14:57:06 2025 by DylibTool
#ifndef LIBZMQ_V143_MT_4_3_6_IMPORTS_H
#define LIBZMQ_V143_MT_4_3_6_IMPORTS_H

#include "Win32Defs.h"

#ifndef STATIC_LIBLIBZMQ_V143_MT_4_3_6
// Disable function declarations in the original header
// file by #defining function names to point to symbols that
// are not referenced by any code.
#define zmq_atomic_counter_dec __1d1e7041_zmq_atomic_counter_dec
#define zmq_atomic_counter_destroy __72770433_zmq_atomic_counter_destroy
#define zmq_atomic_counter_inc __161e704c_zmq_atomic_counter_inc
#define zmq_atomic_counter_new __1d0a704b_zmq_atomic_counter_new
#define zmq_atomic_counter_set __1d097056_zmq_atomic_counter_set
#define zmq_atomic_counter_value __19110536_zmq_atomic_counter_value
#define zmq_bind __18041f3b_zmq_bind
#define zmq_close __7c011e2c_zmq_close
#define zmq_connect __7c616b31_zmq_connect
#define zmq_connect_peer __0c040e1c_zmq_connect_peer
#define zmq_ctx_destroy __0f130374_zmq_ctx_destroy
#define zmq_ctx_get __7e7c7d00_zmq_ctx_get
#define zmq_ctx_get_ext __1b04095f_zmq_ctx_get_ext
#define zmq_ctx_new __777c7e00_zmq_ctx_new
#define zmq_ctx_set __6a7c7d00_zmq_ctx_set
#define zmq_ctx_set_ext __0f04095f_zmq_ctx_set_ext
#define zmq_ctx_shutdown __0e1e0b1a_zmq_ctx_shutdown
#define zmq_ctx_term __6d7c7b6d_zmq_ctx_term
#define zmq_curve_keypair __77370925_zmq_curve_keypair
#define zmq_curve_public __1e2b1a3f_zmq_curve_public
#define zmq_device __7d6d0736_zmq_device
#define zmq_disconnect __121e6c59_zmq_disconnect
#define zmq_errno __701f0331_zmq_errno
#define zmq_getsockopt __021f6e43_zmq_getsockopt
#define zmq_has __120c025f_zmq_has
#define zmq_init __1303182b_zmq_init
#define zmq_join __10021831_zmq_join
#define zmq_leave __73081029_zmq_leave
#define zmq_msg_close __11727973_zmq_msg_close
#define zmq_msg_copy __74716679_zmq_msg_copy
#define zmq_msg_data __737f6261_zmq_msg_data
#define zmq_msg_get __707b6200_zmq_msg_get
#define zmq_msg_gets __707b6273_zmq_msg_gets
#define zmq_msg_group __006c7975_zmq_msg_group
#define zmq_msg_init __7e707f74_zmq_msg_init
#define zmq_msg_init_buffer __47777812_zmq_msg_init_buffer
#define zmq_msg_init_data __40141e00_zmq_msg_init_data
#define zmq_msg_init_size __4403160e_zmq_msg_init_size
#define zmq_msg_more __7a716465_zmq_msg_more
#define zmq_msg_move __7a716065_zmq_msg_move
#define zmq_msg_recv __657b7576_zmq_msg_recv
#define zmq_msg_routing_id __657b042b_zmq_msg_routing_id
#define zmq_msg_send __647b7864_zmq_msg_send
#define zmq_msg_set __647b6200_zmq_msg_set
#define zmq_msg_set_group __73090d2a_zmq_msg_set_group
#define zmq_msg_set_routing_id __161e7074_zmq_msg_set_routing_id
#define zmq_msg_size __64776c65_zmq_msg_size
#define zmq_poll __0a021d33_zmq_poll
#define zmq_poller_add __0b144252_zmq_poller_add
#define zmq_poller_add_fd __6f141d34_zmq_poller_add_fd
#define zmq_poller_destroy __657a3625_zmq_poller_destroy
#define zmq_poller_fd __0b704255_zmq_poller_fd
#define zmq_poller_modify __79142b38_zmq_poller_modify
#define zmq_poller_modify_fd __794b4d5c_zmq_poller_modify_fd
#define zmq_poller_new __0a07425d_zmq_poller_new
#define zmq_poller_remove __6f1d2d37_zmq_poller_remove
#define zmq_poller_remove_fd __6f424b53_zmq_poller_remove_fd
#define zmq_poller_size __060a2740_zmq_poller_size
#define zmq_poller_wait __0e193644_zmq_poller_wait
#define zmq_poller_wait_all __6f755a1b_zmq_poller_wait_all
#define zmq_ppoll __661d1e33_zmq_ppoll
#define zmq_proxy __731f1e27_zmq_proxy
#define zmq_proxy_steerable __74497a32_zmq_proxy_steerable
#define zmq_recv __08081229_zmq_recv
#define zmq_recviov __61676429_zmq_recviov
#define zmq_recvmsg __657b7529_zmq_recvmsg
#define zmq_send __09081f3b_zmq_send
#define zmq_send_const __251f7055_zmq_send_const
#define zmq_sendiov __6067693b_zmq_sendiov
#define zmq_sendmsg __647b783b_zmq_sendmsg
#define zmq_setsockopt __161f6e43_zmq_setsockopt
#define zmq_sleep __7901143a_zmq_sleep
#define zmq_socket __6c761234_zmq_socket
#define zmq_socket_get_peer_state __7a130108_zmq_socket_get_peer_state
#define zmq_socket_monitor __6c6a242d_zmq_socket_monitor
#define zmq_socket_monitor_pipes_stats __2e1a6a4f_zmq_socket_monitor_pipes_stats
#define zmq_socket_monitor_versioned __66766d56_zmq_socket_monitor_versioned
#define zmq_stopwatch_intermediate __7343182e_zmq_stopwatch_intermediate
#define zmq_stopwatch_start __77556d38_zmq_stopwatch_start
#define zmq_stopwatch_stop __79571938_zmq_stopwatch_stop
#define zmq_strerror __7b6b6c48_zmq_strerror
#define zmq_term __0e080332_zmq_term
#define zmq_threadclose __00120556_zmq_threadclose
#define zmq_threadstart __0e13044e_zmq_threadstart
#define zmq_timers_add __1813435b_zmq_timers_add
#define zmq_timers_cancel __7119203c_zmq_timers_cancel
#define zmq_timers_destroy __767d372c_zmq_timers_destroy
#define zmq_timers_execute __7077202a_zmq_timers_execute
#define zmq_timers_new __19004354_zmq_timers_new
#define zmq_timers_reset __1904263c_zmq_timers_reset
#define zmq_timers_set_interval __01161552_zmq_timers_set_interval
#define zmq_timers_timeout __606e2621_zmq_timers_timeout
#define zmq_unbind __61671336_zmq_unbind
#define zmq_version __65676d2c_zmq_version
#define zmq_z85_decode __0055276f_zmq_z85_decode
#define zmq_z85_encode __015e276f_zmq_z85_encode

#define __declspec(x)
#endif // STATIC_LIBLIBZMQ_V143_MT_4_3_6

#include "zmq.h"

#ifndef STATIC_LIBLIBZMQ_V143_MT_4_3_6
#undef __declspec
// Use #undef to restore function names before declaring
// function pointers with the names originally used to
// declare imports.
#undef zmq_atomic_counter_dec
extern decltype(&__1d1e7041_zmq_atomic_counter_dec) zmq_atomic_counter_dec;
#undef zmq_atomic_counter_destroy
extern decltype(&__72770433_zmq_atomic_counter_destroy) zmq_atomic_counter_destroy;
#undef zmq_atomic_counter_inc
extern decltype(&__161e704c_zmq_atomic_counter_inc) zmq_atomic_counter_inc;
#undef zmq_atomic_counter_new
extern decltype(&__1d0a704b_zmq_atomic_counter_new) zmq_atomic_counter_new;
#undef zmq_atomic_counter_set
extern decltype(&__1d097056_zmq_atomic_counter_set) zmq_atomic_counter_set;
#undef zmq_atomic_counter_value
extern decltype(&__19110536_zmq_atomic_counter_value) zmq_atomic_counter_value;
#undef zmq_bind
extern decltype(&__18041f3b_zmq_bind) zmq_bind;
#undef zmq_close
extern decltype(&__7c011e2c_zmq_close) zmq_close;
#undef zmq_connect
extern decltype(&__7c616b31_zmq_connect) zmq_connect;
#undef zmq_connect_peer
//extern decltype(&__0c040e1c_zmq_connect_peer) zmq_connect_peer;
#undef zmq_ctx_destroy
extern decltype(&__0f130374_zmq_ctx_destroy) zmq_ctx_destroy;
#undef zmq_ctx_get
extern decltype(&__7e7c7d00_zmq_ctx_get) zmq_ctx_get;
#undef zmq_ctx_get_ext
//extern decltype(&__1b04095f_zmq_ctx_get_ext) zmq_ctx_get_ext;
#undef zmq_ctx_new
extern decltype(&__777c7e00_zmq_ctx_new) zmq_ctx_new;
#undef zmq_ctx_set
extern decltype(&__6a7c7d00_zmq_ctx_set) zmq_ctx_set;
#undef zmq_ctx_set_ext
//extern decltype(&__0f04095f_zmq_ctx_set_ext) zmq_ctx_set_ext;
#undef zmq_ctx_shutdown
extern decltype(&__0e1e0b1a_zmq_ctx_shutdown) zmq_ctx_shutdown;
#undef zmq_ctx_term
extern decltype(&__6d7c7b6d_zmq_ctx_term) zmq_ctx_term;
#undef zmq_curve_keypair
extern decltype(&__77370925_zmq_curve_keypair) zmq_curve_keypair;
#undef zmq_curve_public
extern decltype(&__1e2b1a3f_zmq_curve_public) zmq_curve_public;
#undef zmq_device
extern decltype(&__7d6d0736_zmq_device) zmq_device;
#undef zmq_disconnect
extern decltype(&__121e6c59_zmq_disconnect) zmq_disconnect;
#undef zmq_errno
extern decltype(&__701f0331_zmq_errno) zmq_errno;
#undef zmq_getsockopt
extern decltype(&__021f6e43_zmq_getsockopt) zmq_getsockopt;
#undef zmq_has
extern decltype(&__120c025f_zmq_has) zmq_has;
#undef zmq_init
extern decltype(&__1303182b_zmq_init) zmq_init;
#undef zmq_join
//extern decltype(&__10021831_zmq_join) zmq_join;
#undef zmq_leave
//extern decltype(&__73081029_zmq_leave) zmq_leave;
#undef zmq_msg_close
extern decltype(&__11727973_zmq_msg_close) zmq_msg_close;
#undef zmq_msg_copy
extern decltype(&__74716679_zmq_msg_copy) zmq_msg_copy;
#undef zmq_msg_data
extern decltype(&__737f6261_zmq_msg_data) zmq_msg_data;
#undef zmq_msg_get
extern decltype(&__707b6200_zmq_msg_get) zmq_msg_get;
#undef zmq_msg_gets
extern decltype(&__707b6273_zmq_msg_gets) zmq_msg_gets;
#undef zmq_msg_group
//extern decltype(&__006c7975_zmq_msg_group) zmq_msg_group;
#undef zmq_msg_init
extern decltype(&__7e707f74_zmq_msg_init) zmq_msg_init;
#undef zmq_msg_init_buffer
//extern decltype(&__47777812_zmq_msg_init_buffer) zmq_msg_init_buffer;
#undef zmq_msg_init_data
extern decltype(&__40141e00_zmq_msg_init_data) zmq_msg_init_data;
#undef zmq_msg_init_size
extern decltype(&__4403160e_zmq_msg_init_size) zmq_msg_init_size;
#undef zmq_msg_more
extern decltype(&__7a716465_zmq_msg_more) zmq_msg_more;
#undef zmq_msg_move
extern decltype(&__7a716065_zmq_msg_move) zmq_msg_move;
#undef zmq_msg_recv
extern decltype(&__657b7576_zmq_msg_recv) zmq_msg_recv;
#undef zmq_msg_routing_id
//extern decltype(&__657b042b_zmq_msg_routing_id) zmq_msg_routing_id;
#undef zmq_msg_send
extern decltype(&__647b7864_zmq_msg_send) zmq_msg_send;
#undef zmq_msg_set
extern decltype(&__647b6200_zmq_msg_set) zmq_msg_set;
#undef zmq_msg_set_group
//extern decltype(&__73090d2a_zmq_msg_set_group) zmq_msg_set_group;
#undef zmq_msg_set_routing_id
//extern decltype(&__161e7074_zmq_msg_set_routing_id) zmq_msg_set_routing_id;
#undef zmq_msg_size
extern decltype(&__64776c65_zmq_msg_size) zmq_msg_size;
#undef zmq_poll
extern decltype(&__0a021d33_zmq_poll) zmq_poll;
#undef zmq_poller_add
//extern decltype(&__0b144252_zmq_poller_add) zmq_poller_add;
#undef zmq_poller_add_fd
//extern decltype(&__6f141d34_zmq_poller_add_fd) zmq_poller_add_fd;
#undef zmq_poller_destroy
//extern decltype(&__657a3625_zmq_poller_destroy) zmq_poller_destroy;
#undef zmq_poller_fd
//extern decltype(&__0b704255_zmq_poller_fd) zmq_poller_fd;
#undef zmq_poller_modify
//extern decltype(&__79142b38_zmq_poller_modify) zmq_poller_modify;
#undef zmq_poller_modify_fd
//extern decltype(&__794b4d5c_zmq_poller_modify_fd) zmq_poller_modify_fd;
#undef zmq_poller_new
//extern decltype(&__0a07425d_zmq_poller_new) zmq_poller_new;
#undef zmq_poller_remove
//extern decltype(&__6f1d2d37_zmq_poller_remove) zmq_poller_remove;
#undef zmq_poller_remove_fd
//extern decltype(&__6f424b53_zmq_poller_remove_fd) zmq_poller_remove_fd;
#undef zmq_poller_size
//extern decltype(&__060a2740_zmq_poller_size) zmq_poller_size;
#undef zmq_poller_wait
//extern decltype(&__0e193644_zmq_poller_wait) zmq_poller_wait;
#undef zmq_poller_wait_all
//extern decltype(&__6f755a1b_zmq_poller_wait_all) zmq_poller_wait_all;
#undef zmq_ppoll
//extern decltype(&__661d1e33_zmq_ppoll) zmq_ppoll;
#undef zmq_proxy
extern decltype(&__731f1e27_zmq_proxy) zmq_proxy;
#undef zmq_proxy_steerable
extern decltype(&__74497a32_zmq_proxy_steerable) zmq_proxy_steerable;
#undef zmq_recv
extern decltype(&__08081229_zmq_recv) zmq_recv;
#undef zmq_recviov
extern decltype(&__61676429_zmq_recviov) zmq_recviov;
#undef zmq_recvmsg
extern decltype(&__657b7529_zmq_recvmsg) zmq_recvmsg;
#undef zmq_send
extern decltype(&__09081f3b_zmq_send) zmq_send;
#undef zmq_send_const
extern decltype(&__251f7055_zmq_send_const) zmq_send_const;
#undef zmq_sendiov
extern decltype(&__6067693b_zmq_sendiov) zmq_sendiov;
#undef zmq_sendmsg
extern decltype(&__647b783b_zmq_sendmsg) zmq_sendmsg;
#undef zmq_setsockopt
extern decltype(&__161f6e43_zmq_setsockopt) zmq_setsockopt;
#undef zmq_sleep
extern decltype(&__7901143a_zmq_sleep) zmq_sleep;
#undef zmq_socket
extern decltype(&__6c761234_zmq_socket) zmq_socket;
#undef zmq_socket_get_peer_state
//extern decltype(&__7a130108_zmq_socket_get_peer_state) zmq_socket_get_peer_state;
#undef zmq_socket_monitor
extern decltype(&__6c6a242d_zmq_socket_monitor) zmq_socket_monitor;
#undef zmq_socket_monitor_pipes_stats
//extern decltype(&__2e1a6a4f_zmq_socket_monitor_pipes_stats) zmq_socket_monitor_pipes_stats;
#undef zmq_socket_monitor_versioned
//extern decltype(&__66766d56_zmq_socket_monitor_versioned) zmq_socket_monitor_versioned;
#undef zmq_stopwatch_intermediate
extern decltype(&__7343182e_zmq_stopwatch_intermediate) zmq_stopwatch_intermediate;
#undef zmq_stopwatch_start
extern decltype(&__77556d38_zmq_stopwatch_start) zmq_stopwatch_start;
#undef zmq_stopwatch_stop
extern decltype(&__79571938_zmq_stopwatch_stop) zmq_stopwatch_stop;
#undef zmq_strerror
extern decltype(&__7b6b6c48_zmq_strerror) zmq_strerror;
#undef zmq_term
extern decltype(&__0e080332_zmq_term) zmq_term;
#undef zmq_threadclose
extern decltype(&__00120556_zmq_threadclose) zmq_threadclose;
#undef zmq_threadstart
extern decltype(&__0e13044e_zmq_threadstart) zmq_threadstart;
#undef zmq_timers_add
extern decltype(&__1813435b_zmq_timers_add) zmq_timers_add;
#undef zmq_timers_cancel
extern decltype(&__7119203c_zmq_timers_cancel) zmq_timers_cancel;
#undef zmq_timers_destroy
extern decltype(&__767d372c_zmq_timers_destroy) zmq_timers_destroy;
#undef zmq_timers_execute
extern decltype(&__7077202a_zmq_timers_execute) zmq_timers_execute;
#undef zmq_timers_new
extern decltype(&__19004354_zmq_timers_new) zmq_timers_new;
#undef zmq_timers_reset
extern decltype(&__1904263c_zmq_timers_reset) zmq_timers_reset;
#undef zmq_timers_set_interval
extern decltype(&__01161552_zmq_timers_set_interval) zmq_timers_set_interval;
#undef zmq_timers_timeout
extern decltype(&__606e2621_zmq_timers_timeout) zmq_timers_timeout;
#undef zmq_unbind
extern decltype(&__61671336_zmq_unbind) zmq_unbind;
#undef zmq_version
extern decltype(&__65676d2c_zmq_version) zmq_version;
#undef zmq_z85_decode
extern decltype(&__0055276f_zmq_z85_decode) zmq_z85_decode;
#undef zmq_z85_encode
extern decltype(&__015e276f_zmq_z85_encode) zmq_z85_encode;

#endif // STATIC_LIBLIBZMQ_V143_MT_4_3_6

namespace Dylib { bool libzmq_v143_mt_4_3_6_Loaded(); }

#endif // LIBZMQ_V143_MT_4_3_6_IMPORTS_H
