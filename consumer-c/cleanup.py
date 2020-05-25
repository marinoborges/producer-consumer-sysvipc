#!/usr/bin/python2
# FILENAME:
#   cleanup.py
# DESCRIPTION :
#   Clean SysV IPC shared memory and semaphores used in the project
# AUTHOR:
#   Marino Borges
# CREATED:
#   05.17.2020
# CHANGES:
#   05.19.2020 producer-consumer-sysvipc-0.4
#       - added verbose feature

# Python modules
import argparse

# 3rd party modules
import sysv_ipc

# Set argparser and get args
parser = argparse.ArgumentParser(description='Clean SysV IPC shared memory and semaphores used in the project')
parser.add_argument('-v','--verbose', action='store_true', help='set verbose flag (default: false)')
args = parser.parse_args()
verbose = args.verbose

def semaphore_remove(key):
    try:
        semaphore = sysv_ipc.Semaphore(key)
    except sysv_ipc.ExistentialError:
        if (verbose):
            print('''The semaphore with key "{}" doesn't exist.'''.format(key))
    else:
        semaphore.remove()
        if (verbose):
            print('Removed the semaphore with key "{}".'.format(key))

def memory_remove(key):
    try:
        memory = sysv_ipc.SharedMemory(key)
    except sysv_ipc.ExistentialError:
        if (verbose):
            print('''The shared memory with key "{}" doesn't exist.'''.format(key))
    else:
        memory.remove()
        if (verbose):
            print('Removed the shared memory with key "{}".'.format(key))


content_key = 42
contentsize_key = 44
semaphore_remove(content_key)
semaphore_remove(contentsize_key)
memory_remove(content_key)
memory_remove(contentsize_key)
