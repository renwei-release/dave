# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */


class t_print_progress_bar():
    self_percentage = 0
    self_total_process = 0
    self_current_process = 0

    def __init__(self, total_process, message=None):
        self.self_percentage = 10000
        self.self_total_process = total_process
        self.self_current_process = 0
        if message is not None:
            print(message)

    def show(self):
        self.self_current_process += 1
        if self.self_total_process <= self.self_current_process:
            percentage = 100
        else:
            percentage = int((int(self.self_current_process) * 100) / int(self.self_total_process))
        if self.self_percentage != percentage:
            self.self_percentage = percentage
            for step in range(0, percentage + 1):
                print('\r[%3d%%] %s' % (step, '>' * step), flush=True, end='')
        if percentage == 100:
            print(f'')
        if percentage > 100:
            print(f'{self.self_current_process} bar overflow!')


def t_print_class_member(class_struct):
    print(f"======== t_print_class_member:{class_struct} =======")
    print([e for e in dir(class_struct) if not e.startswith('_')])
    print(f"\n\nvars:{vars(class_struct)}")
    return