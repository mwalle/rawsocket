raw sockets made easy
=====================

Only in linux, sorry.


Rationale
---------

If you need raw sockets in your python script, you have to run it as root
or make it SUID root. Alternatively, you can use file capabilities to grant
extended priviliges to a binary. Unfortunately, this only works on binaries
and not on scripts, which means you have to set the capability on the
python binary.


How does it work?
-----------------

As mentioned above, you can only set capabilties on a binary. Therefore,
this module has a helper binary, which will get the RAW_NET capability.
This binary is spawned by the rawsocket python module every time you create
a new raw socket. The helper uses its capability to create the socket and
then passes the file descriptor back to the calling program.

In linux you can pass file descriptors through unix domain sockets by using
the ``sendmsg()`` system call and ``SCM_RIGHTS``. See also `fd passing`_.


Installation
------------

Use PIP::

  pip install rawsocket

Or use the standard method::

  python setup.py install

This will install the extension module and the helper binary. But it will
*not* set the file capability on the helper binary. You have two choices:

1. Grant every user access to the helper (simple, *not recommended*)::

     setcap cap_net_raw+ep rawsocket-helper

2. Grant access only to users which are member of a specific group
   (recommended)::

     chmod 750 rawsocket-helper
     chown root:yourgroup rawsocket-helper
     setcap cap_net_raw+ep rawsocket-helper


Example
-------

.. code:: python

  import socket, rawsocket
  fd = rawsocket.rawsocket_fd()
  s = socket.fromfd(fd, socket.AF_PACKET, socket.SOCK_RAW)

You see easy as pie ;)


License
-------

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

.. _fd passing: http://keithp.com/blogs/fd-passing/
