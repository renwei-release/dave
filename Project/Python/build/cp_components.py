# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2025 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import ast
import os
import sys
import shutil


def _get_imports_from_file(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        node = ast.parse(f.read(), filename=file_path)
    imports = []
    for n in ast.walk(node):
        if isinstance(n, ast.Import):
            for alias in n.names:
                imports.append(alias.name.replace('.', '/'))
        elif isinstance(n, ast.ImportFrom):
            if n.module:
                imports.append(n.module.replace('.', '/'))
    return imports


def _get_path_py_list(path):
    py_list = []
    for root, dirs, files in os.walk(path):
        for filename in files:
            if os.path.isfile(os.path.join(root, filename)):
                if filename.endswith(".py"):
                   py_list.append(os.path.join(root, filename))
    return py_list


def _collect_components_imports(product_home):
    components_imports = []

    py_list = _get_path_py_list(product_home)

    for py_file in py_list:
        imports = _get_imports_from_file(py_file)
        components = [imp for imp in imports if imp.startswith('components')]
        components_imports.extend(components)

    return components_imports


def _copy_components(all_imports, project_home, copy_components_dest):
    if copy_components_dest == None:
        return

    if not os.path.exists(copy_components_dest):
        os.makedirs(copy_components_dest)

    for imp in all_imports:
        src_file = f"{project_home}/{imp}"

        if os.path.isfile(f'{src_file}.py'):
            src_file = f'{src_file}.py'
            rel_path = os.path.relpath(src_file, start=project_home)
            dest_file = os.path.join(copy_components_dest, rel_path)
            dest_dir = os.path.dirname(dest_file)
            if not os.path.exists(dest_dir):
                os.makedirs(dest_dir, exist_ok=True)
            shutil.copy2(src_file, dest_file)
        else:
            src_dir = src_file
            rel_path = os.path.relpath(src_dir, start=project_home)
            dest_dir = os.path.join(copy_components_dest, rel_path)
            if not os.path.exists(dest_dir):
                shutil.copytree(src_dir, dest_dir)

    if copy_components_dest and not os.path.exists(os.path.join(copy_components_dest, 'components')):
        os.makedirs(os.path.join(copy_components_dest, 'components'))

    return


if __name__ == '__main__':
    project_name = sys.argv[1]
    if len(sys.argv) > 2:
        copy_components_dest = sys.argv[2]
    else:
        copy_components_dest = None

    project_home='../../../../project'
    product_home=f"{project_home}/product/{project_name}"

    all_imports = _collect_components_imports(product_home)
    _copy_components(all_imports, project_home, copy_components_dest)