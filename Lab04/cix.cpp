// $Id: cix.cpp,v 1.9 2019-04-05 15:04:28-07 - - $
// Trystan Nguyen trtanguy
// Leonard Tolentino lemtolen

#include <iostream>
#include <fstream> 
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream outlog (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit" , cix_command::EXIT},
   {"help" , cix_command::HELP},
   {"ls"   , cix_command::LS  },
   {"get"  , cix_command::GET },
   {"put"  , cix_command::PUT },
   {"rm"   , cix_command::RM  },
};

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
   cout << help;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if (header.command != cix_command::LSOUT) {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }else {
      auto buffer = make_unique<char[]> (header.nbytes + 1);
      recv_packet (server, buffer.get(), header.nbytes);
      outlog << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer.get();
   }
}

//---------------------------------
//New Function Zone

//Send Header
//Expects Header on Fail
//Expects Header and Payload on successs
void cix_get(client_socket& server, string fn = ""){
   //Check Use
   if (fn.compare("") == 0){
      outlog << "Usage: get filename" << endl;
      return;
   }
   if (fn.find('/') != string::npos){
      outlog << "Usage: file must be in directory" << endl;
      return;
   }

   //Get Header
   cix_header header;
   header.command = cix_command::GET;
   fn.copy(header.filename, fn.size());

   //Send Header
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;

   //Server Response Fail
   if (header.command != cix_command::FILEOUT) {
      outlog << "sent GET, server did not return FILEOUT" << endl;
      outlog << "server returned " << header << endl;
   }
   //Server Response Success
   else{
      //Recieve Buffer
      char* buffer = new char[FILENAME_SIZE];
      recv_packet (server, buffer, header.nbytes);
      outlog << "received " << header.nbytes << " bytes" << endl;

      //Write File
      ofstream outFile (fn.c_str(), ofstream::binary);
      if (outFile.fail()) {
         outlog << "Unable to Open " << fn 
            << ": " << strerror (errno) << endl;
         return;
      }
      outFile.write(buffer, (header.nbytes));
      outFile.close();
      delete[] buffer;
   }
}

//Sends Header and Payload
//Expects Header on Fail
//Expects Header on Success
void cix_put(client_socket& server, string fn = ""){
   //Check Use
   if (fn.compare("") == 0){
      outlog << "Usage: put filename" << endl;
      return;
   }
   if (fn.find('/') != string::npos){
      outlog << "Usage: file must be in directory" << endl;
      return;
   }

   //Open File
   ifstream inFile (fn, ifstream::binary);
   if (inFile.fail()) {
      outlog << "Unable to Open " << fn.c_str() 
         << ": " << strerror (errno) << endl;
      return;
   }

   //Write to Char*
   char* write = new char[FILENAME_SIZE];
   inFile.read (write, FILENAME_SIZE);
   write[inFile.gcount()] = '\0';

   //Write Header
   cix_header header;
   header.command = cix_command::PUT;
   fn.copy(header.filename, fn.size());
   header.filename[fn.size()] = '\0';
   header.nbytes = inFile.gcount();

   //Send Header and Payload
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   outlog << "sending " << inFile.gcount() << " bytes" << endl;
   send_packet (server, write, inFile.gcount());
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   delete[] write;

   //Server Response Fail
   if (header.command != cix_command::ACK) {
      outlog << "sent GET, server did not return ACK" << endl;
      outlog << "server returned " << header << endl;
   }
   //Server Response Success
   else{
      outlog << "PUT successful on " << fn << endl;
      outlog << "server returned " << header << endl;
   }
}

//Sends Header
//Expects Header on Fail
//Expects Header
void cix_rm(client_socket& server, string fn = ""){
   //Check Filename
   if (fn.compare("") == 0){
      outlog << "Usage: rm filename" << endl;
      return;
   }
   if (fn.find('/') != string::npos){
      outlog << "Usage: file must be in directory" << endl;
      return;
   }

   //Get Header
   cix_header header;
   header.command = cix_command::RM;
   fn.copy(header.filename, fn.size()+1);
   header.filename[fn.size()] = '\0';

   //Send Header
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;

   //Server Response Fail
   if (header.command != cix_command::ACK) {
      outlog << "sent RM, server did not return ACK" << endl;
      outlog << "server returned " << header << endl;
   }
   //Server Response Success
   else{
      outlog << "RM successful on " << fn << endl;
      outlog << "server returned " << header << endl;
   }
}
//-------------------------------

void usage() {
   cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   outlog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   outlog << to_string (hostinfo()) << endl;
   try {
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      outlog << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         outlog << "command " << line << endl;

         int sec = line.find(' ') + 1;
         string arg1;
         if (sec != 0) arg1 = line.substr(0, sec-1);
         else arg1 = line;
         const auto& itor = command_map.find (arg1);
         
         cix_command cmd = itor == command_map.end()
                         ? cix_command::ERROR : itor->second;

         switch (cmd) {
            case cix_command::EXIT:
               throw cix_exit();
               break;
            case cix_command::HELP:
               cix_help();
               break;
            case cix_command::LS:
               cix_ls (server);
               break;
            case cix_command::GET:           //Inserted
               if (sec != 0) cix_get (server, line.substr(sec));
               else cix_get (server);
               break;
            case cix_command::PUT:
               if (sec != 0) cix_put (server, line.substr(sec));
               else cix_put (server);
               break;
            case cix_command::RM:
               if (sec != 0) cix_rm (server, line.substr(sec));
               else cix_rm (server);
               break;                       //^
            default:
               outlog << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cix_exit& error) {
      outlog << "caught cix_exit" << endl;
   }
   outlog << "finishing" << endl;
   return 0;
}

