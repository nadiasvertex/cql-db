# Runs the unit tests automatically everytime a change is made. Requires a Linux kernel with
# inotify capability. 

import os
import pyinotify
import subprocess
import sys

class OnWriteHandler(pyinotify.ProcessEvent):
    def my_init(self, cwd, extension, cmd):
        self.cwd = cwd
        self.extensions = extension.split(',')
        self.cmd = cmd

    def _run_cmd(self):
        print '==> Modification detected'
        subprocess.call(self.cmd.split(' '), cwd=self.cwd)

    def process_IN_MODIFY(self, event):
        if all(not event.pathname.endswith(ext) for ext in self.extensions):
            return
        self._run_cmd()

def auto_compile(path, extension, cmd):
    wm = pyinotify.WatchManager()
    handler = OnWriteHandler(cwd=path, extension=extension, cmd=cmd)
    notifier = pyinotify.Notifier(wm, default_proc_fun=handler)
    wm.add_watch(path, pyinotify.ALL_EVENTS, rec=True, auto_add=True)
    print '==> Start monitoring %s (type c^c to exit)' % path
    notifier.loop()

if __name__ == '__main__':
    
    path = os.getcwd()    
    extension = ".cpp,.h,.proto"    
    cmd = '/bin/sh test.sh'
    
    # Blocks monitoring
    auto_compile(path, extension, cmd)
