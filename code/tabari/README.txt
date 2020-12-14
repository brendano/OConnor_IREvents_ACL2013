Notes by Brendan, 2020-12-13

This code was used to run TABARI for experiments.
I don't think any of these experiments were part of the ACL 2013 paper.
There are several binary versions of TABARI here.
The one used in the tabari_wrap.sh script was done successfully.

Since the executable file type says Mach-O, that implies I ran them on my
Intel mac laptop around March 2013.  It was probably with whatever version of
OS X that was current then.

Unfortunately I can't find an example of the input format my scripts used.
They reformat it into the input that TABARI expected.
That format is pretty simple and described in TABARI's documentation.
I checked in a copy into docs/.

Like all the other code in this repository, it assumes Python 2.7.

Some of the scripts:

tabari_wrap.sh - actually runs TABARI, setting up a "project" file that
controls TABARI's configuration, including input/output files.

ssplit2tabari.py - outputs the TABARI input format. It's just a line of
metadata, then a sentence, etc.

tabari2tuple.py - translates TABARI's output into the semantic tuple format
used internally by our larger research codebase
