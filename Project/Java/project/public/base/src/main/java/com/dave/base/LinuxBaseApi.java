package com.dave.base;

import com.sun.jna.Callback;
import com.sun.jna.Library;
import com.sun.jna.Pointer;

public interface LinuxBaseApi extends Library {
    // callback typedefs mapped to JNA Callback
    interface dll_callback_fun extends Callback { void invoke(Pointer msg); }
    interface dll_checkback_fun extends Callback { int invoke(int value); }
    interface dll_kv_timerout_fun extends Callback { void invoke(Pointer kv, Pointer key); }
    interface dll_cfg_reg_fun extends Callback { void invoke(Pointer name_ptr, int name_len, Pointer value_ptr, int value_len); }
    interface dll_dos_cmd_fun extends Callback { int invoke(Pointer param_ptr, int param_len); }
    interface dll_timer_fun extends Callback { void invoke(long timer_id, long thread_index); }

    void dave_dll_init(String product_name,
                       String my_verno, String work_mode,
                       int thread_number,
                       dll_callback_fun init_fun, dll_callback_fun main_fun, dll_callback_fun exit_fun,
                       String sync_domain);

    void dave_dll_running();
    void dave_dll_exit();
    int dave_dll_run_state();

    int dave_dll_self_check(String string_data, int int_data, float float_data, dll_checkback_fun checkback);

    void dave_dll_log(String func, int line, String log_msg, int log_type);

    String dave_dll_verno();
    String dave_dll_reset_verno(String verno);
    String dave_dll_my_gid();

    Pointer dave_dll_mmalloc(int length, String fun, int line);
    int dave_dll_mfree(Pointer m, String fun, int line);
    Pointer dave_dll_mclone(Pointer m, String fun, int line);

    long dave_dll_thread_id(String thread_name);
    long dave_dll_gid_id(String gid, String thread_name);
    String dave_dll_self();

    Pointer dave_dll_thread_msg(int msg_len, String fun, int line);
    void dave_dll_thread_msg_release(Pointer ptr, String fun, int line);

    int dave_dll_thread_id_msg(long dst_id, int msg_id, int msg_len, Pointer msg_body, String fun, int line);
    int dave_dll_thread_id_qmsg(long dst_id, int msg_id, int msg_len, Pointer msg_body, String fun, int line);
    Pointer dave_dll_thread_id_co(long dst_id, int req_id, int req_len, Pointer req_body, int rsp_id, String fun, int line);
    Pointer dave_dll_thread_id_qco(long dst_id, int req_id, int req_len, Pointer req_body, int rsp_id, String fun, int line);

    int dave_dll_thread_name_msg(String dst_thread, int msg_id, int msg_len, Pointer msg_body, String fun, int line);
    int dave_dll_thread_name_qmsg(String dst_thread, int msg_id, int msg_len, Pointer msg_body, String fun, int line);
    Pointer dave_dll_thread_name_co(String dst_thread, int req_id, int req_len, Pointer req_body, int rsp_id, String fun, int line);
    Pointer dave_dll_thread_name_qco(String dst_thread, int req_id, int req_len, Pointer req_body, int rsp_id, String fun, int line);

    int dave_dll_thread_gid_msg(String gid, String dst_thread, int msg_id, int msg_len, Pointer msg_body, String fun, int line);
    int dave_dll_thread_gid_qmsg(String gid, String dst_thread, int msg_id, int msg_len, Pointer msg_body, String fun, int line);
    Pointer dave_dll_thread_gid_co(String gid, String dst_thread, int req_id, int req_len, Pointer req_body, int rsp_id, String fun, int line);
    Pointer dave_dll_thread_gid_qco(String gid, String dst_thread, int req_id, int req_len, Pointer req_body, int rsp_id, String fun, int line);

    int dave_dll_thread_uid_msg(String uid, int msg_id, int msg_len, Pointer msg_body, String fun, int line);
    int dave_dll_thread_uid_qmsg(String uid, int msg_id, int msg_len, Pointer msg_body, String fun, int line);
    Pointer dave_dll_thread_uid_co(String uid, int req_id, int req_len, Pointer req_body, int rsp_id, String fun, int line);
    Pointer dave_dll_thread_uid_qco(String uid, int req_id, int req_len, Pointer req_body, int rsp_id, String fun, int line);

    Pointer dave_dll_thread_name_sync_msg(String dst_thread, int req_id, int req_len, Pointer req_body, int rsp_id, int rsp_len, Pointer rsp_body, String fun, int line);
    Pointer dave_dll_thread_id_sync_msg(long dst_id, int req_id, int req_len, Pointer req_body, int rsp_id, int rsp_len, Pointer rsp_body, String fun, int line);

    int dave_dll_thread_broadcast_msg(String thread_name, int msg_id, int msg_len, Pointer msg_body, String fun, int line);

    int dave_dll_cfg_set(String cfg_name, String cfg_value);
    int dave_dll_cfg_get(String cfg_name, Pointer cfg_value_ptr, int cfg_value_len);
    int dave_dll_cfg_del(String cfg_name);
    int dave_dll_cfg_reg(String cfg_name, dll_cfg_reg_fun reg_fun);
    int dave_dll_cfg_remote_set(String cfg_name, String cfg_value, int ttl);
    int dave_dll_cfg_remote_get(String cfg_name, Pointer cfg_value_ptr, int cfg_value_len);
    int dave_dll_cfg_remote_del(String cfg_name);

    void dave_dll_poweroff();

    Pointer dave_dll_kv_malloc(String name, int out_second, dll_kv_timerout_fun outback_fun);
    Pointer dave_dll_kv_remote_malloc(String name, int out_second, dll_kv_timerout_fun outback_fun);
    void dave_dll_kv_free(Pointer kv);
    int dave_dll_kv_add(Pointer kv, String key, String value);
    int dave_dll_kv_inq(Pointer kv, String key, Pointer value_ptr, int value_len);
    int dave_dll_kv_del(Pointer kv, String key);

    int dave_dll_dos_cmd_reg(String cmd, dll_dos_cmd_fun cmd_fun);
    void dave_dll_dos_print(String msg);
    String dave_dll_dos_get_user_input(String give_user_msg, int wait_second);

    void dave_dll_system_online();
    void dave_dll_system_offline();

    int dave_dll_timer_creat(String name, int alarm_second, dll_timer_fun timer_fun);
    void dave_dll_timer_kill(String name);
}
