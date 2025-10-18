package com.dave.base;

import com.sun.jna.Native;

import java.io.*;
import java.net.URL;
import java.nio.file.Files;
import java.security.DigestInputStream;
import java.security.MessageDigest;
import java.util.Locale;

public final class NativeLoader {
    private NativeLoader() {}

    public static LinuxBaseApi load() {
        String res = "/native/liblinuxBASE.so";
        URL url = NativeLoader.class.getResource(res);
        try (InputStream in = NativeLoader.class.getResourceAsStream(res)) {
            if (in == null) {
                throw new IllegalStateException("native lib not found: " + res);
            }
            if (url != null) {
                System.err.println("[NativeLoader] Native resource URL: " + url);
            }
            File dir = new File(System.getProperty("java.io.tmpdir"), "base-native");
            if (!dir.exists()) dir.mkdirs();
            File so = File.createTempFile("liblinuxBASE", ".so", dir);
            so.deleteOnExit();

            // 计算资源内容的 SHA-256，帮助定位是否加载了旧版本
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            try (DigestInputStream din = new DigestInputStream(in, md);
                 OutputStream out = new FileOutputStream(so)) {
                din.transferTo(out);
            }
            String sha256 = hex(md.digest());
            try { so.setReadable(true); so.setExecutable(true); } catch (Throwable ignored) {}

            System.err.println("[NativeLoader] Extracted native to: " + so.getAbsolutePath()
                    + ", size=" + Files.size(so.toPath()) + " bytes, sha256=" + sha256);
            return Native.load(so.getAbsolutePath(), LinuxBaseApi.class);
        } catch (IOException e) {
            throw new RuntimeException("failed to extract native lib", e);
        } catch (Exception e) {
            throw new RuntimeException("failed to prepare native lib", e);
        }
    }

    private static String hex(byte[] b) {
        StringBuilder sb = new StringBuilder(b.length * 2);
        for (byte x : b) sb.append(String.format(Locale.ROOT, "%02x", x));
        return sb.toString();
    }
}
