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


input_dir='/project/product/test/test/unit_test/input'
output_dir='/project/product/test/test/unit_test/output'
test_case_name='test_case'


def _load_service_testcase(suite, service_table):
    for service_name in service_table.keys():
        service_name = service_name.decode('utf-8')

        if os.path.exists(f'/project/product/test/test/unit_test/service/{service_name}/{test_case_name}.py') == True:
            try:
                test_case = importlib.import_module(f'product.test.test.unit_test.service.{service_name}.{test_case_name}')
                test_case.test_case(suite, input_dir, output_dir)
                DAVELOG(f'add test case:\t{service_name}')
            except:
                traceback.print_exc()
    return


def _html_testrunner():
    fp = open(f'{output_dir}/unit_test.html', 'wb')
    return HTMLTestRunner(stream=fp, title=u'unit test', description=u'Test report for DAVE service')


# =====================================================================


def unit_test(service_table):
    suite = unittest.TestSuite()

    _load_service_testcase(suite, service_table)

    runner = _html_testrunner()

    runner.run(suite)

    dave_poweroff()
    return