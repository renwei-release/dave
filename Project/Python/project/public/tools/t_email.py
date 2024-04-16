# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from .t_crypto import t_crypto_base64_encode


def _filename_to_mimetype(filename):
    mimetype = 'application/octet-stream'
    if filename.endswith('.txt'):
        mimetype = 'text/plain'
    elif filename.endswith('.pdf'):
        mimetype = 'application/pdf'
    elif filename.endswith('.doc'):
        mimetype = 'application/msword'
    elif filename.endswith('.docx'):
        mimetype = 'application/vnd.openxmlformats-officedocument.wordprocessingml.document'
    elif filename.endswith('.xls'):
        mimetype = 'application/vnd.ms-excel'
    elif filename.endswith('.xlsx'):
        mimetype = 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'
    elif filename.endswith('.ppt'):
        mimetype = 'application/vnd.ms-powerpoint'
    elif filename.endswith('.pptx'):
        mimetype = 'application/vnd.openxmlformats-officedocument.presentationml.presentation'
    elif filename.endswith('.jpg'):
        mimetype = 'image/jpeg'
    elif filename.endswith('.jpeg'):
        mimetype = 'image/jpeg'
    elif filename.endswith('.png'):
        mimetype = 'image/png'
    elif filename.endswith('.gif'):
        mimetype = 'image/gif'
    elif filename.endswith('.bmp'):
        mimetype = 'image/bmp'
    elif filename.endswith('.zip'):
        mimetype = 'application/zip'
    elif filename.endswith('.rar'):
        mimetype = 'application/x-rar-compressed'
    elif filename.endswith('.tar'):
        mimetype = 'application/x-tar'
    elif filename.endswith('.gz'):
        mimetype = 'application/gzip'
    elif filename.endswith('.7z'):
        mimetype = 'application/x-7z-compressed'
    elif filename.endswith('.mp3'):
        mimetype = 'audio/mpeg'
    elif filename.endswith('.wav'):
        mimetype = 'audio/wav'
    elif filename.endswith('.wma'):
        mimetype = 'audio/x-ms-wma'
    elif filename.endswith('.mp4'):
        mimetype = 'video/mp4'
    elif filename.endswith('.avi'):
        mimetype = 'video/x-msvideo'
    elif filename.endswith('.wmv'):
        mimetype = 'video/x-ms-wmv'
    elif filename.endswith('.flv'):
        mimetype = 'video/x-flv'
    elif filename.endswith('.mkv'):
        mimetype = 'video/x-matroska'
    elif filename.endswith('.mov'):
        mimetype = 'video/quicktime'
    elif filename.endswith('.rmvb'):
        mimetype = 'application/vnd.rn-realmedia-vbr'
    elif filename.endswith('.rm'):
        mimetype = 'application/vnd.rn-realmedia'
    elif filename.endswith('.webm'):
        mimetype = 'video/webm'
    elif filename.endswith('.ogg'):
        mimetype = 'application/ogg'
    elif filename.endswith('.ogv'):
        mimetype = 'video/ogg'
    elif filename.endswith('.oga'):
        mimetype = 'audio/ogg'
    elif filename.endswith('.ogx'):
        mimetype = 'application/ogg'
    elif filename.endswith('.ogm'):
        mimetype = 'application/ogg'
    return mimetype


# =====================================================================


def t_email_attachment(filename):
    with open(filename, 'rb') as f:
        filebody = f.read()
        if filebody:
            return {
                'filename': filename,
                'mimetype': _filename_to_mimetype(filename),
                'filebody': t_crypto_base64_encode(filebody)
            }
    return None