# Producer-Consumer-SysVIPC

Producer (lang=Python) write a file content into shared memory area
Consumer (lang=C) read shared memory area and write to file
- Program uses 2 shared memory areas: one for content size and another for the content itself
- Option to check MD5 hash
**Installation**
1. Install packages: python2.7 python-sysv-ipc gcc
2. Compile consumer:
```
<PATH>/consumer-c$ ./make_all.sh
```
3. Run Consumer first
4. Run Producer passing argument inputfile

**Producer usage**
```
usage: producer.py [-h] [-m] [-v] inputfile

Write file in shared memory segment using SysV IPC (check md5 hash optionally)

positional arguments:
  inputfile        input file (required)

optional arguments:
  -h, --help       show this help message and exit
  -m, --md5-check  set check MD5 flag (default: false)
  -v, --verbose    set verbose flag (default: false)
```

**Consumer usage**
```
Usage: ./consumer [-h] [-v] [-m]

Read shared memory segment using SysV IPC (check MD5 hash optionally)
Write file with shared memory content

optional arguments:
        -h      show this help message and exit
        -v      set verbose flag (default: false)
        -m      set check MD5 flag (default: false)
```

Additional notes:
- File producer-consumer-sysvipc/producer-py/cleanup.py cleans shared memory and semaphores IDs if being used
- Consumer C program catches signals SIGINT (ctrl-c key press) and SIGTERM (kill command) in order to finish gracefully
- Useful bash commands to get current usage of shared memory and semaphore
```
# BASH command: list shared memory and semaphore
# ipcs -m
# ipcs -s

# BASH command: remove shared memory and semaphore
# ipcrm -m <ID>
# ipcrm -s <ID>
```
- Consumer may be run in background:
```
$ ./consumer &
```
- Ensure consumer and producer are running with matched arguments for check MD5 flag
- File type set to ".jpg" by default. Set variable filetype to modify it
- File basename set to "file" by default. Output filename will be set with a incremental file counter suffix
- Semaphores and shared memory keys can be changed. In order to do that, edit producer.py, consumer.c and cleanup.py corresponding variables. Consumer needs to be recompiled.
- producer.py works with Python2 as well as Python3, but Python2 has better execution times