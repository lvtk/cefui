#!/usr/bin/env python

import os, platform, subprocess
os.environ['LV2_PATH'] = os.path.join (os.getcwd(), 'build/stage/lib/lv2')
os.environ['LD_LIBRARY_PATH'] = os.path.join (os.getcwd(), 'build/stage/lib')
cmd = ['jalv.gtk', 'http://lvtoolkit.org/plugins/cefamp']
#cmd = ['ingen', '-eg']
subprocess.call (cmd)
