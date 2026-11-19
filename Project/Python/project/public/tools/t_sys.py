# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import threading
import inspect
import os
import sys
import subprocess
import getpass
import socket
import ipaddress
import time
import random


def t_sys_user_name():
    username = getpass.getuser()
    return username


def t_sys_pc_name():
    hostname = os.popen('hostname').read().replace('\r', '').replace('\n', '')
    return hostname


def t_sys_path_append(path=None):
    current_file = sys._getframe(1).f_code.co_filename
    current_file_absolute_path = os.path.dirname(os.path.abspath(current_file))
    if path != None:
        sys.path.append(current_file_absolute_path+path)
    else:
        sys.path.append(current_file_absolute_path)
    return


def t_sys_path_show():
    print(f"python package search path:{sys.path}")
    return


def t_sys_path_file_number(path):
    file_number = 0
    file_list = os.listdir(path)
    for file_name in file_list:
        if os.path.isdir(os.path.join(path, file_name)):
            file_number += t_sys_path_file_number(os.path.join(path, file_name))
        else:
            file_number += 1
    return file_number


def t_sys_gpu_name():
    try:
        result = subprocess.run(['nvidia-smi', '--query-gpu=name', '--format=csv,noheader'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        if result.returncode != 0:
            print(f"nvidia-smi command failed with error: {result.stderr}")
            return 'CPU'

        gpu_models = result.stdout.strip().split('\n')
        return gpu_models[0]
    except FileNotFoundError:
        return 'CPU'


def t_sys_mypath():
    if hasattr(sys, '_MEIPASS'):
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        temp_dir = sys._MEIPASS
    elif len(sys.argv) >= 3:
        # Fallback to the current directory if not running as a PyInstaller bundle
        # command = [main_program, auto_argument, sys._MEIPASS] + sys.argv[1:]
        temp_dir = sys.argv[2]
    else:
        # Fallback to the current directory if not running as a PyInstaller bundle
        temp_dir = '/project'

    if not os.path.isdir(temp_dir):
        return '/project'

    return temp_dir


def t_sys_myip():
    # Prefer the outbound interface IP that would be used to reach external hosts.
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.connect(('8.8.8.8', 80))
            outbound_ip = sock.getsockname()[0]
            if outbound_ip and not outbound_ip.startswith('127.'):
                return outbound_ip
    except OSError:
        pass

    try:
        result = subprocess.run(['hostname', '-I'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        if result.returncode != 0:
            print(f"hostname command failed with error: {result.stderr}")
            return 'localhost'

        ip_candidates = result.stdout.strip().split()
        valid_ips = []

        for ip_text in ip_candidates:
            try:
                ip_obj = ipaddress.ip_address(ip_text)
            except ValueError:
                continue

            if ip_obj.is_loopback or ip_obj.is_link_local or ip_obj.is_multicast or ip_obj.is_unspecified:
                continue

            valid_ips.append(ip_obj)

        for ip_obj in valid_ips:
            if ip_obj.version == 4 and not ip_obj.is_private:
                return str(ip_obj)

        for ip_obj in valid_ips:
            if ip_obj.version == 4:
                return str(ip_obj)

        for ip_obj in valid_ips:
            if ip_obj.version == 6 and getattr(ip_obj, 'is_global', False):
                return str(ip_obj)

        if valid_ips:
            return str(valid_ips[0])

        return 'localhost'
    except FileNotFoundError:
        return 'localhost'


def t_sys_attributes():
    attributes = dir(sys)
    return attributes


def t_sys_memory_used_ratio():
    mem = os.popen('free')
    mem = mem.readlines()

    mem = mem[1].split()

    memory_ratio = int(mem[2]) / int(mem[1])
    return memory_ratio


def t_sys_memory_free_ratio():
    mem = os.popen('free')
    mem = mem.readlines()

    mem = mem[1].split()

    memory_ratio = int(mem[3]) / int(mem[1])
    return memory_ratio


def t_sys_memory_free_volume():
    mem = os.popen('free')
    mem = mem.readlines()

    mem = mem[1].split()

    memory_volume = int(mem[3]) / (1024 * 1024)
    return memory_volume


def t_sys_memory_cache_ratio():
    mem = os.popen('free')
    mem = mem.readlines()

    mem = mem[1].split()

    memory_ratio = int(mem[6]) / int(mem[1])
    return memory_ratio

#
# depth=2 is the caller of the caller, 
# which is the function that calls the function that calls t_sys_myline. 
# This is usually the function that we want to log.
#
def t_sys_myline(depth=2):
    current_depth = len(inspect.stack())
    if depth >= current_depth:
        return 'NULL'.encode("utf-8"), 0
    __func__ = sys._getframe(depth).f_code.co_name.encode("utf-8")
    __LINE__ = sys._getframe(depth).f_lineno
    return __func__, __LINE__


def t_sys_the_port_is_listening(port):
    try:
        port = int(port)
    except ValueError:
        return False

    retry_times = 3
    retry_interval_seconds = 0.5

    for i in range(retry_times):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            if s.connect_ex(('127.0.0.1', port)) == 0:
                return True

        if i < (retry_times - 1):
            time.sleep(retry_interval_seconds)

    # Add a randomized delay before final confirmation to reduce transient
    # false negatives in clustered/containerized environments.
    final_check_delay_seconds = random.uniform(0.2, 1.0)
    time.sleep(final_check_delay_seconds)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        if s.connect_ex(('127.0.0.1', port)) == 0:
            return True

    return False


def t_sys_choose_free_port(port):
    from public.base.dave_log import DAVELOG

    while True:
        time.sleep(random.uniform(10, 60))
        if t_sys_the_port_is_listening(port) == False:
            break
        DAVELOG(f"Port:{port} is in use, trying next port...")
        time.sleep(random.uniform(30, 60))
        port += 1
    return port


def t_sys_delay_reboot(seconds=6):
    from public.base.dave_log import DAVEABNORMAL

    fun, line = t_sys_myline()
    DAVEABNORMAL(f"Exiting process in {seconds} seconds for container restart. Called from function: {fun.decode('utf-8')}, line: {line}")
    timer = threading.Timer(seconds, os._exit, args=(1,))
    timer.daemon = True
    timer.start()