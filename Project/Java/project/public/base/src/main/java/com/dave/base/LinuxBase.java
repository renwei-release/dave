package com.dave.base;

import com.sun.jna.Memory;
import com.sun.jna.Platform;
import com.sun.jna.Pointer;

import java.nio.charset.StandardCharsets;
import java.util.Objects;

public final class LinuxBase {
    private static final LinuxBaseApi API;
    static {
        if (!Platform.isLinux()) {
            throw new UnsupportedOperationException("liblinuxBASE.so only supported on Linux");
        }
        API = NativeLoader.load();
    }

    private LinuxBase() {}

    public interface InitCallback { void onCall(Pointer ctx); }
    public interface MainCallback { void onCall(Pointer ctx); }
    public interface ExitCallback { void onCall(Pointer ctx); }

    private static LinuxBaseApi.dll_callback_fun initHolder;
    private static LinuxBaseApi.dll_callback_fun mainHolder;
    private static LinuxBaseApi.dll_callback_fun exitHolder;

    public static void init(String productName,
                            String version,
                            String workMode,
                            int threadNumber,
                            InitCallback initCb,
                            MainCallback mainCb,
                            ExitCallback exitCb,
                            String syncDomain) {
        Objects.requireNonNull(productName, "productName");
        Objects.requireNonNull(version, "version");
        Objects.requireNonNull(workMode, "workMode");
        Objects.requireNonNull(syncDomain, "syncDomain");

        initHolder = (ctx) -> { if (initCb != null) initCb.onCall(ctx); };
        mainHolder = (ctx) -> { if (mainCb != null) mainCb.onCall(ctx); };
        exitHolder = (ctx) -> { if (exitCb != null) exitCb.onCall(ctx); };

        API.dave_dll_init(productName, version, workMode, threadNumber,
                initHolder, mainHolder, exitHolder, syncDomain);
    }

    public static void running() { API.dave_dll_running(); }
    public static void exit() { API.dave_dll_exit(); }
    public static int runState() { return API.dave_dll_run_state(); }

    public static int selfCheck(String stringData, int intData, float floatData, java.util.function.IntUnaryOperator checkback) {
        LinuxBaseApi.dll_checkback_fun cb = (v) -> checkback == null ? 0 : checkback.applyAsInt(v);
        return API.dave_dll_self_check(stringData, intData, floatData, cb);
    }

    public static void log(String func, int line, String msg, int type) { API.dave_dll_log(func, line, msg, type); }
    public static String verno() { return API.dave_dll_verno(); }
    public static String resetVerno(String verno) { return API.dave_dll_reset_verno(verno); }
    public static String myGid() { return API.dave_dll_my_gid(); }
    public static String self() { return API.dave_dll_self(); }

    public static Pointer mmalloc(int length, String fun, int line) { return API.dave_dll_mmalloc(length, fun, line); }
    public static int mfree(Pointer m, String fun, int line) { return API.dave_dll_mfree(m, fun, line); }
    public static Pointer mclone(Pointer m, String fun, int line) { return API.dave_dll_mclone(m, fun, line); }

    public static long threadId(String threadName) { return API.dave_dll_thread_id(threadName); }
    public static long gidId(String gid, String threadName) { return API.dave_dll_gid_id(gid, threadName); }

    public static int threadIdMsg(long dstId, int msgId, byte[] body, String fun, int line) {
        int len = body == null ? 0 : body.length;
        Pointer p = null;
        try {
            if (len > 0) { p = new Memory(len); p.write(0, body, 0, len); }
            return API.dave_dll_thread_id_msg(dstId, msgId, len, p, fun, line);
        } finally { }
    }

    public static int threadNameMsg(String dst, int msgId, byte[] body, String fun, int line) {
        int len = body == null ? 0 : body.length;
        Pointer p = null;
        try {
            if (len > 0) { p = new Memory(len); p.write(0, body, 0, len); }
            return API.dave_dll_thread_name_msg(dst, msgId, len, p, fun, line);
        } finally { }
    }

    public static int cfgSet(String name, String value) { return API.dave_dll_cfg_set(name, value); }
    public static String cfgGet(String name, int maxLen) {
        Memory buf = new Memory(Math.max(1, maxLen));
        int rc = API.dave_dll_cfg_get(name, buf, (int) buf.size());
        if (rc < 0) return null;
        return buf.getString(0, StandardCharsets.UTF_8.name());
    }
    public static int cfgDel(String name) { return API.dave_dll_cfg_del(name); }
}
