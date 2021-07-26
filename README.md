# star : Simple tar

GNU `tar` is an archiving program designed to store multiple files in a single file (an archive), and to manipulate such archives. This is a simplified version of `tar`.

It can **archive** files under one directory into one file, or can **extract** the files from the archived file.

## How to build and use

```
$ gcc -o star star.c
$ ./star archive <archive-file-name> <target-directory-path>
$ ./star extract <archive-file-name>
```