./dns-proxy port# [disableChecks] where: 
   port#   :   Is a port with listen permissions (ie 10010)
   disableChecks   :   An optional parameter.  If specified, it disables the corresponding checks in BEC - 00000xyz
      1 (0000 0001) Disables TML check
      2 (0000 0010) Disables checksum check
      4 (0000 0100) Disables magic number check
      7 (0000 0111) disables all three checks.

examples:
./dns-proxy 10010
    Runs DNS proxy and listens on UDP port 10010.  All checking is enabled.

./dns-proxy 10010 2
    Runs DNS proxy with checksum check disabled.  It will still throw a warning if the client sends an invalid checksum.

./dns-proxy 10010 3
    Runs the server with checksum check (2) and TML check (1) disabled.




Use the following to send the sample client request to test a server.

cat clientPacket | nc -u localhost 10010 

   cat clientPacket - reads the file and prints to stdout
   pipe (|) redirects stdout to next command's stdin
   nc -u localhost 10010 reads stdin and sends the data as a packet to the specified host/port.
      -u represents UDP mode.

