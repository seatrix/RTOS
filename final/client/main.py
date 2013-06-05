import socket
import sys

# Parse arguments
if len(sys.argv) > 1:

   listip = ""
   port = 55555
   sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
   sock.bind((listip, port))

   # Discover address of WiflyClock
   data, addr = sock.recvfrom(1024)
   listip = addr[0]
   port = addr[1]
   print "Discovered WiflyClock!"
   print "ip:   ", listip
   print "port: ", port

   # Change screen color (TRY SENDING 0ACED0)
   if sys.argv[1] == 'Color':
      if len(sys.argv) == 3:
         osock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
         CONSTR = "COL" + sys.argv[2]
         print CONSTR
         osock.sendto(CONSTR, (listip, 2000))
         osock.close();
      else:
         print "Insufficient arguments"

   # Toggle on/off
   elif sys.argv[1] == 'Toggle':
      if len(sys.argv) == 2:
         osock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
         CONSTR = "TOG"
         print CONSTR
         osock.sendto(CONSTR, (listip, 2000))
         osock.close();
      else:
         print "Insufficient arguments"

   # Write line
   elif sys.argv[1] == 'Write':
      if len(sys.argv) == 4:
         osock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
         CONSTR = "WRT" + sys.argv[2] + sys.argv[3]
         print CONSTR
         osock.sendto(CONSTR, (listip, 2000))
         osock.close();
      else:
         print "Insufficient arguments"

   # Clear Screen
   elif sys.argv[1] == 'Clear':
      if len(sys.argv) == 2:
         osock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
         CONSTR = "CLR"
         print CONSTR
         osock.sendto(CONSTR, (listip, 2000))
         osock.close();
      else:
         print "Insufficient arguments"

   else:
      print "Unkown command!"

   sock.close()

else:
   print "Usage: wiflyclk [cmd]"
