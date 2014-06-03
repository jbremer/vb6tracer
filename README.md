VB6 Tracer
==========

VB6Tracer is a tool designed to instrument the Visual Basic 6 Virtual Machine
in order to analyze VB6 P-Code applications at runtime.

Usage
-----

VB6Tracer has been designed to work in two different ways. It can either be
ran manually from the commandline using the dllinject utility, or it can be
ran automatically by integrating it with a recent version of Cuckoo Sandbox.

Use at your own risk!
---------------------

At all times use the tool in a Virtual Machine. We are not responsible for
any damage. Furthermore, the tool might not work etc :-)

Manual Usage
------------

Running the following command from cmd.exe should do the trick. It will then
write a log file to `C:\logz.txt`.

```bash
$ utils/dllinject.exe vb6tracer.dll sample.exe
```

Automated Usage
---------------

To modify an existing Cuckoo installation to work with VB6 Tracer there are
two steps involved.

* Overwrite Cuckoo's monitor DLL with vb6tracer.dll
  * Copy vb6tracer.dll to `$CUCKOO/analyzer/windows/dll/cuckoomon.dll`
* Modify Cuckoo to not randomize pipe names in the Guest.
  * Open `$CUCKOO/analyzer/windows/lib/common/constants.py`
  * Replace the `PIPE =` line by `PIPE = "\\\\.\\PIPE\\cuckoo"`

To then analyze a sample and get its log, follow the following steps.

* Submit your sample to Cuckoo (`./utils/submit.py sample.exe`.)
* Run the Cuckoo daemon (`./cuckoo.py -d`, `-d` for debug mode.)
* Wait for the analysis to finish.
* Open the logz.txt file, which can be found under
  `$CUCKOO/storage/analyses/latest/files/*/logz.txt`

Compilation
-----------

To compile the DLL under Ubuntu you'll need the `mingw32` package and run
`make`. Provided are two DLLs, one `generic` and one `specific`. The `generic`
vb6tracer DLL dumps mnemonics of all the executed instructions. The `specific`
DLL dumps meta-information of only a subset of instructions which seemed
interesting at the time of developing this tool.

To modify this behavior please open up `main.c` and comment or uncomment the
`vb6_hook_generic(...)` and `vb6_set_hooks()` lines.

License
-------

VB6Tracer is licensed under GPLv3.

Authors
-------

Jurriaan Bremer, Marion Marschalek.
