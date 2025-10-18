/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import com.dave.base.LinuxBaseApi;
import com.dave.base.NativeLoader;
import com.sun.jna.Platform;
import com.sun.jna.Pointer;

public class Main {
    // 初始化参数（可根据需求调整或改为从配置加载）
    private static final String PRODUCT_NAME = resolveProductName();
    // 获取 pom.xml 中的 ${project.version}（优先从 Manifest 的 Implementation-Version；退化到 pom.properties）
    private static final String VERSION = resolveVersion();
    private static final String WORK_MODE = "Outer Loop";
    private static final int THREAD_NUMBER = Math.max(1, Runtime.getRuntime().availableProcessors());
    private static final String SYNC_DOMAIN = "";

    // 回调实现，保持强引用，避免 GC
    private static final LinuxBaseApi.dll_callback_fun INIT_CB = new LinuxBaseApi.dll_callback_fun() {
        @Override public void invoke(Pointer msg) {
            System.out.println("[INIT_CB] msg=" + msg);
        }
    };
    private static final LinuxBaseApi.dll_callback_fun MAIN_CB = new LinuxBaseApi.dll_callback_fun() {
        @Override public void invoke(Pointer msg) {
            System.out.println("[MAIN_CB] msg=" + msg);
        }
    };
    private static final LinuxBaseApi.dll_callback_fun EXIT_CB = new LinuxBaseApi.dll_callback_fun() {
        @Override public void invoke(Pointer msg) {
            System.out.println("[EXIT_CB] msg=" + msg);
        }
    };

    public static void main(String[] args) {
        System.out.println("OS=" + System.getProperty("os.name") + ", ARCH=" + System.getProperty("os.arch"));
        if (!Platform.isLinux()) {
            System.err.println("Host OS is not Linux, liblinuxBASE.so only loads on Linux.");
            return;
        }

        try {
            LinuxBaseApi api = NativeLoader.load();
            System.out.println("liblinuxBASE.so loaded. Calling dave_dll_init(...)");
            api.dave_dll_init(
                    PRODUCT_NAME, VERSION, WORK_MODE, THREAD_NUMBER,
                    INIT_CB, MAIN_CB, EXIT_CB, SYNC_DOMAIN);

            System.out.println("dave_dll_init() invoked.");

            api.dave_dll_running();
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Native link error: " + e.getMessage());
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }

    private static String resolveVersion() {
        // 1) 从 Manifest: Implementation-Version
        try {
            Package pkg = Main.class.getPackage();
            if (pkg != null) {
                String impl = pkg.getImplementationVersion();
                if (impl != null && !impl.isBlank()) return impl;
            }
        } catch (Throwable ignored) {}

        // 2) 从 Maven 生成的 pom.properties
        try (java.io.InputStream in = Main.class.getResourceAsStream("/META-INF/maven/com.example/BASE/pom.properties")) {
            if (in != null) {
                java.util.Properties p = new java.util.Properties();
                p.load(in);
                String v = p.getProperty("version");
                if (v != null && !v.isBlank()) return v;
            }
        } catch (Throwable ignored) {}

        // 3) 兜底：环境变量或占位
        String env = System.getenv("PROJECT_VERSION");
        if (env != null && !env.isBlank()) return env;
        return "unknown";
    }

    private static String resolveProductName() {
        // 1) Manifest: Implementation-Title（我们在 pom 中把 artifactId 写进了这个条目）
        try {
            Package pkg = Main.class.getPackage();
            if (pkg != null) {
                String title = pkg.getImplementationTitle();
                if (title != null && !title.isBlank()) return title;
            }
        } catch (Throwable ignored) {}

        // 2) Maven pom.properties 中的 artifactId
        try (java.io.InputStream in = Main.class.getResourceAsStream("/META-INF/maven/com.example/BASE/pom.properties")) {
            if (in != null) {
                java.util.Properties p = new java.util.Properties();
                p.load(in);
                String a = p.getProperty("artifactId");
                if (a != null && !a.isBlank()) return a;
            }
        } catch (Throwable ignored) {}

        // 3) 环境变量覆盖
        String env = System.getenv("PROJECT_ARTIFACT_ID");
        if (env != null && !env.isBlank()) return env;

        // 4) 兜底
        return "BASE";
    }
}
