m4api is a tool for monitoring and configuring Mini-Box M4-ATX power supplies.

Built using libusb 0.1, m4api can be used on Linux, Mac, BSD and Windows
systems. For information about libusb, see http://www.libusb.org/.

Up-to-date source code is available at:
  http://ram.umd.edu/wiki/Public/Software/m4api
  or http://ram.umd.edu/git/

For Git access:
  $ git clone git://ram.umd.edu/bits/m4api.git

USAGE
=====

$ ./m4 -diag [loop]         # Print diagnostic values
$ ./m4 -config              # Print all config values
$ ./m4 -config FIELD        # Print one field
$ ./m4 -config FIELD VALUE  # Set one value

-- Ken Tossell
   ktossell@umd.edu

