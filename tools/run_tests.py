#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, platform, subprocess
os.environ['LV2_PATH'] = os.path.join (os.getcwd(), 'build/stage/lib/lv2')
os.environ['LD_LIBRARY_PATH'] = os.path.join (os.getcwd(), 'build/stage/lib')
cmd = ['build/stage/bin/tests']
subprocess.call (cmd)
