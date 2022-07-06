# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import unittest
import sys
import traceback
import os
from product.test.tools.HTMLTestRunner import *
from public import *


input_dir='/project/product/test/test/service_test/input'
output_dir='/project/product/test/test/service_test/output'
test_case_name='test_case'


def _load_service_testcase(suite, gid, service_name, service_id):
    service_name = service_name.decode('utf-8')

    if os.path.exists(f'/project/product/test/test/service_test/service/{service_name}/{test_case_name}.py') == True:
        try:
            test_case = importlib.import_module(f'product.test.test.service_test.service.{service_name}.{test_case_name}')
            test_case.test_case(suite, input_dir, output_dir, gid, service_name, service_id)
            DAVELOG(f'add test case:\t{service_name}')
            return True
        except:
            traceback.print_exc()
    return False


def _html_testrunner(gid, service_name, service_id):
    gid = gid.decode('utf-8')
    service_name = service_name.decode('utf-8')
    service_id = "{:x}".format(service_id)

    fp = open(f'{output_dir}/service_{gid}_{service_name}_{service_id}.html', 'wb')
    return HTMLTestRunner(stream=fp, title=u'service test', description=u'Test report for DAVE service')


# =====================================================================


def service_test(gid, service_name, service_id):
    suite = unittest.TestSuite()

    if _load_service_testcase(suite, gid, service_name, service_id) == True:
        runner = _html_testrunner(gid, service_name, service_id)
        runner.run(suite)
    return