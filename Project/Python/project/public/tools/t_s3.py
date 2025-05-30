# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from .t_rand import t_rand
import os
import re
try:
    import boto3
    from botocore.client import Config
except ImportError:
    pass


__S3_MY_LOCAL_PATH__ = '/dave/ftp'


class S3Clinet():
    def __init__(self, s3_config):
        if s3_config == None:
            return
        end_point = s3_config.get("EndPoint", None)
        if end_point == None:
            return
        ak = s3_config.get("AccessKey", None)
        if ak == None:
            return
        sk = s3_config.get("SecretKey", None)
        if sk == None:
            return

        self.client = boto3.client(
                    's3',
                    endpoint_url=end_point,  # MinIO 服务器地址
                    aws_access_key_id=ak,   # MinIO 访问密钥
                    aws_secret_access_key=sk,  # MinIO 秘密密钥
                    region_name='us-east-1',               # 可以是任意区域
                    config=Config(signature_version='s3v4')  # 签名版本
                )

    def _parse_link(self, file_url):
        if file_url.startswith('obs://'):
            match = re.match(r"obs://([^/]+)/(.+)", file_url)

            if match:
                bucket_name = match.group(1)  # 提取存储桶名称
                object_path = match.group(2)  # 提取对象路径
                return bucket_name, object_path
            else:
                return None, None
        else:
            return None, None

    def is_file_exist(self, file_url):
        bucket_name, object_name = self._parse_link(file_url)
        if bucket_name == None:
            return False

        try:
            self.client.head_object(Bucket=bucket_name, Key=object_name)
            return True
        except Exception as _:
            return False

    def is_valid_path(self, path):
        bucket, prefix = self._parse_link(path)
        if bucket is None:
            return False
        try:
            response = self.client.list_objects_v2(Bucket=bucket, Prefix=prefix, MaxKeys=1)
            return response.get("KeyCount", 0) > 0
        except Exception:
            return False

    def download_file(self, file_url, file_path):
        bucket_name, object_name = self._parse_link(file_url)
        if bucket_name == None:
            return False
        
        try:
            self.client.download_file(bucket_name, object_name, file_path)
            return True
        except Exception as _:
            return False
        
    def upload_file(self, file_path, file_url):
        bucket_name, object_name = self._parse_link(file_url)
        if bucket_name == None:
            return False
        try:
            self.client.upload_file(file_path, bucket_name, object_name)
            return True
        except Exception as _:
            return False


# =====================================================================


def t_s3_is_s3_path(path):
    return path.startswith('obs://') or path.startswith('s3://') or path.startswith('oss://') or path.startswith('cos://')


def t_s3_is_valid_file(file, s3_config=None):
    if not t_s3_is_s3_path(file):
        return False

    s3_client = S3Clinet(s3_config)
    return s3_client.is_file_exist(file)


def t_s3_is_valid_path(path, s3_config=None):
    if not t_s3_is_s3_path(path):
        return False

    path = path[:path.rfind('/') + 1]

    s3_client = S3Clinet(s3_config)
    return s3_client.is_valid_path(path)


def t_s3_input_file_download(input_file, s3_config=None):
    if t_s3_is_s3_path(input_file):
        if not os.path.exists(__S3_MY_LOCAL_PATH__):
            os.makedirs(__S3_MY_LOCAL_PATH__)
        input_download_file = f"{__S3_MY_LOCAL_PATH__}/{t_rand()}_{os.path.basename(input_file)}"
        s3_client = S3Clinet(s3_config)
        s3_client.download_file(input_file, input_download_file)
    else:
        input_download_file = input_file
    return input_download_file


def t_s3_input_file_remove(download_file):
    if download_file.startswith(__S3_MY_LOCAL_PATH__):
        os.remove(download_file)
    return


def t_s3_output_file_change(output_file):
    if t_s3_is_s3_path(output_file):
        output_local_file = f"{__S3_MY_LOCAL_PATH__}/{t_rand()}_{os.path.basename(output_file)}"
    else:
        output_local_file = output_file
    return output_local_file


def t_s3_output_file_upload(output_local_file, output_file, s3_config=None):
    if t_s3_is_s3_path(output_file):
        s3_client = S3Clinet(s3_config)
        s3_client.upload_file(output_local_file, output_file)
        os.remove(f"{output_local_file}")
    return