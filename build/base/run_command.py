#!/usr/bin/env python
import subprocess
import re

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

class CommonPolicy(Policy):
    def __init__(self, cb):
        Policy.__init__(self, cb)

class CmakePolicy(Policy):
    def __init__(self, cb):
        Policy.__init__(self, cb)

    def process(self, message):
        self.progress_ += 1.0
        super(CmakePolicy, self).process(message)

    def update_progress_message(self, progress, message):
        super(CmakePolicy, self).update_progress_message(progress, message)

class MakePolicy(Policy):
    def __init__(self, cb):
        Policy.__init__(self, cb)

    def process(self, message):
        if message.type() != MessageType.MESSAGE:
            super(MakePolicy, self).process(message)
            return

        cur = self.parse_message_to_get_percent(message.message())
        if not cur:
            return

        self.progress_ = cur
        super(MakePolicy, self).process(message)

    def update_progress_message(self, progress, message):
        super(MakePolicy, self).update_progress_message(progress, message)

    def parse_message_to_get_percent(self, message):
        if not message:
            return None

        res = re.search(r'\A\[  (\d+)%\]', message)
        if res != None:
            return float(res.group(1))

        return None

class NinjaPolicy(Policy):
    def __init__(self, cb):
        Policy.__init__(self, cb)

    def process(self, message):
        if message.type() != MessageType.MESSAGE:
            super(NinjaPolicy, self).process(message)
            return

        cur,total = self.parse_message_to_get_range(message.message())
        if not cur and not total:
            return

        self.progress_ = cur / total * 100.0
        super(NinjaPolicy, self).process(message)

    def update_progress_message(self, progress, message):
        super(NinjaPolicy, self).update_progress_message(progress, message)

    def parse_message_to_get_range(self, message):
        if not message:
            return None, None

        res = re.search(r'\A\[(\d+)/(\d+)\]', message)
        if res != None:
            return float(res.group(1)), float(res.group(2))

        return None, None

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