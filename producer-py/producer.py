#!/usr/bin/python2
# FILENAME:
#   producer.py
# DESCRIPTION :
#   Write file content in shared memory using SysV IPC
# AUTHOR:
#   Marino Borges
# CREATED:
#   05.17.2020
# CHANGES:
#   05.18.2020 producer-consumer-sysvipc-0.2
#       - fixed CONTENT_MEMORY_SIZE in 5MB for less memory operations
#   05.18.2020 producer-consumer-sysvipc-0.3
#       - optimized producer.py imports
#   05.19.2020 producer-consumer-sysvipc-0.4
#       - added python3 compatibility
#       - added time spent feature

# Python modules
import time # 0.007s
start = time.time()
import sys # 0.000740051269531s
import argparse # 0.00384187698364s
import os.path # 9.53674316406e-07s

# 3rd party modules
import sysv_ipc # 0.00019383430481s

# Local modules
import utils

PY_MAJOR_VERSION = sys.version_info[0]

# Set argparser and get args
parser = argparse.ArgumentParser(description='Write file content in shared memory using SysV IPC (check md5 hash optionally)')
parser.add_argument('inputfile', type=str, help='input file (required)')
parser.add_argument('-m','--md5-check', action='store_true', help='set check MD5 flag (default: false)')
parser.add_argument('-v','--verbose', action='store_true', help='set verbose flag (default: false)')
args = parser.parse_args()
inputfile = args.inputfile
md5_check = args.md5_check
verbose = args.verbose

if (verbose):
    print("PY_MAJOR_VERSION=%s" % PY_MAJOR_VERSION)

# sysv_ipc parameters
content_semaphore_key = 42;
content_memory_key = 42;
contentsize_semaphore_key = 44;
contentsize_memory_key = 44;
contentsize_memory_size = 10; #in bytes
content_memory_size = 5000000 #in bytes
permissions=int("600", 8);
stderr = 0
# Get content and contentsize to write into memory
try:
    if PY_MAJOR_VERSION > 2:
        f = open(inputfile, "rb")
    else:
        f = open(inputfile, "r")
except IOError:
    print("ERROR: file does not exist (%s)" % inputfile)
    exit(-1)

content_size = os.path.getsize(inputfile)
content = f.read()

# Create contentsize_semaphore & shared memory contentsize_memory.
# If the key is already used, run cleanup.py
try:
    content_semaphore = sysv_ipc.Semaphore(content_semaphore_key, 0)
    contentsize_semaphore = sysv_ipc.Semaphore(contentsize_semaphore_key, 0)
    contentsize_memory = sysv_ipc.SharedMemory(contentsize_memory_key, 0, permissions, contentsize_memory_size)
    content_memory = sysv_ipc.SharedMemory(content_memory_key, 0, permissions, content_memory_size)
except sysv_ipc.ExistentialError:
    print("ERROR: shared memory & semaphores not created yet (run consumer first)")
    exit(-1)

# Acquire semaphores
if (verbose):
    utils.say("acquiring semaphore contentsize_semaphore")
contentsize_semaphore.acquire()
if (verbose):
    utils.say("acquiring semaphore content_semaphore")
content_semaphore.acquire()

# Write content size
if (verbose):
    utils.say("writing contentsize..")
contentsize_memory.write(str(content_size)+'\0')
if (verbose):
    utils.say("releasing semaphore contentsize_semaphore")
contentsize_semaphore.release()

# Write content
if (verbose):
    utils.say("writing content..")
content_memory.write(content)

# Release contentsize_semaphore and content_semaphore
if (verbose):
    utils.say("releasing semaphores content_semaphore (content written)")
content_semaphore.release()

# Read memory - rcvd hash
if (md5_check):
    import hashlib
    # Acquire content_semaphore after hash been written
    if (verbose):
        utils.say("acquiring semaphore content_semaphore (rcv hash)")
    content_semaphore.acquire()
    
    if (verbose):
        utils.say("checking md5 hash ..")
    if PY_MAJOR_VERSION > 2:
        rcvd_hash = content_memory.read()[0:32].decode()
    else:
        rcvd_hash = content_memory.read()[0:32]
    content_hash = hashlib.md5(content).hexdigest()
    if (verbose):
        utils.say("content_hash = %s" % content_hash)
        utils.say("rcvd_hash = %s" % rcvd_hash)

    # What I read must be the md5 of what I wrote or something's gone wrong.
    try:
        assert(rcvd_hash == content_hash)
        if (verbose):
            utils.say("hash matched")
        stderr = 0
    except AssertionError:
        raise AssertionError("ERROR: Hashes don't match\n")
        stderr = -1
    finally: 
        if (verbose):
            utils.say("releasing semaphores content_semaphore (finish)")
        content_semaphore.release()
else:
    if (verbose):
        utils.say("skipping md5 hash..")
    
end = time.time()
if (verbose):
    utils.say("Time spent = %ss" %(end - start))
exit(stderr)
# EOF
