import time
import sys

PY_MAJOR_VERSION = sys.version_info[0]

if PY_MAJOR_VERSION > 2:
    NULL_CHAR = 0
else:
    NULL_CHAR = '\0'


def say(s):
    who = sys.argv[0]
    if who.endswith(".py"):
        who = who[:-3]
    s = "%s@%1.6f: %s" % (who, time.time(), s)
    print(s)