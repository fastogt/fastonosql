#!/usr/bin/env python

import os

class BuildError(Exception):
    def __init__(self, value):
        self.value_ = value
    def __str__(self):
        return self.value_

def read_file_line_by_line(file):
    if not os.path.exists(file):
        raise BuildError('file path: %s not exists' % file)

    file_array = []
    with open(file, "r") as ins:
        for line in ins:
            file_array.append(line.strip())

    return file_array