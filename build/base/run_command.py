#!/usr/bin/env python
import subprocess

class MessageType:
    STATUS = 1
    MESSAGE = 2

class Message(object):
    def __init__(self, message, type):
        self.message_ = message
        self.type_ = type

    def message(self):
        return  self.message_

    def type(self):
        return  self.type_

class Policy(object):
    def __init__(self, cb):
        self.progress_ = 0.0
        self.cb_ = cb

    def process(self, message):
        if self.cb_:
            self.cb_(self.progress_, message)

    def update_progress_message(self, progress, message):
        self.progress_ = progress
        self.process(Message(message, MessageType.STATUS))

def run_command_cb(cmd, policy):
    if not policy:
        policy = Policy
    try:
        policy.update_progress_message(0.0, 'Command {0} started'.format(cmd))
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        while True:
            output = process.stdout.readline()
            if output == '' and process.poll() is not None:
                break
            if output:
                line = output.strip()
                policy.process(Message(line, MessageType.MESSAGE))
        rc = process.poll()
        policy.update_progress_message(100.0, 'Command {0} finished successfully'.format(cmd))
    except subprocess.CalledProcessError as ex:
        policy.update_progress_message(100.0, 'Command {0} finished with exception {1}'.format(cmd, str(ex)))
        raise ex

    return rc