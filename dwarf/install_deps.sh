In the case that your distribution does not ship a working libdwarf(*buntu systems), libdwarf can be found here.

http://www.prevanders.net/libdwarf-20140805.tar.gz

In the case that you require a 32-bit library, add '-m32' to the CFLAGS in the Makefile. You must have libelf installed. On debian-based systems, it will be called libelf-dev.
