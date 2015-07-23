import sys
import os
loaded = False

RTLD_GLOBAL = -1
RTLD_NOW = -1

try:
    from dl import RTLD_GLOBAL, RTLD_NOW
except ImportError:
    osname = os.uname()[0]
    if osname == 'Linux' or osname == 'SunOS':
        RTLD_GLOBAL = 0x100
        RTLD_NOW = 0x2
    elif osname == 'Darwin':
        RTLD_GLOBAL = 0x8
        RTLD_NOW = 0x2

if hasattr(sys, 'setdlopenflags') and -1 not in (RTLD_GLOBAL, RTLD_NOW):
    old_flags = sys.getdlopenflags() 
    sys.setdlopenflags(RTLD_GLOBAL | RTLD_NOW)
    from _pyhp import evaluate, execute
    sys.setdlopenflags(old_flags)
else:
    from _pyhp import evaluate, execute

__all__ = ['evaluate', 'execute']
