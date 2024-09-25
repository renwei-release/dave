# -*- mode: python ; coding: utf-8 -*-
import sys
import os.path as osp

block_cipher = None

a = Analysis(
    ['boot.py'],
    pathex=['.'],
    binaries=[
		('./project/public/base/lib/liblinuxBASE.so', 'public/base/lib')
	],
    datas=[
        ('./project/dave_main.py', '.'),
		('./project/components', 'components'),
		('./project/product/dave_product.py', 'product'),
		('./project/product/___FLAG_FOR_PRODUCT___', 'product/___FLAG_FOR_PRODUCT___'),
		('./project/public', 'public'),
	],
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
    cipher=block_cipher,
)
pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='___FLAG_FOR_BIN___',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)

coll = COLLECT(
    exe,
    a.binaries,
    a.zipfiles,
    a.datas,
    strip=False,
    upx=True,
    upx_exclude=[],
    name='___FLAG_FOR_PRODUCT___',
)