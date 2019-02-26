/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//               Copyright (C) 2011-2017 - The DESY CMS Group                  //
//                           All rights reserved                               //
//                                                                             //
//      The CMStkModLab source code is licensed under the GNU GPL v3.0.        //
//      You have the right to modify and/or redistribute this source code      //
//      under the terms specified in the license, which may be found online    //
//      at http://www.gnu.org/licenses or at License.txt.                      //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#include "ComHandler.h"
#include "BurnInException.h"

#include <sys/select.h>

// SETTINGS ON THE DEVICE:
// (MENU RS-232)
// 
// BAUD: 9600 (default) or set by using the corresponding constructor parameter.
// HARDW HANDSHAKE: on
// PARITY: none (8N1)
// TX TERM: NL

/*!
  The serial port &lt;ioPort&gt; may be specified in several ways:<br><br>
  COM1 ... COM4<br>
  ttyS0 ... ttyS3<br>
  "/dev/ttyS1" ... "/dev/ttyS3"
*/
ComHandler::ComHandler( const ioport_t ioPort, speed_t baud ) {

  // save ioport 
  fIoPort = ioPort;

  // initialize
  OpenIoPort();
  InitializeIoPort( baud );
}

ComHandler::~ComHandler( void ) {

  // restore ioport options as they were
  RestoreIoPort();

  // close device file
  CloseIoPort();
}

//! Send the command string &lt;commandString&gt; to device.
void ComHandler::SendCommand( const char *commandString, bool sendfeed ) {

  char singleCharacter = 0;

  std::cout << "Command to " << fIoPort << ": " << commandString << std::endl;

  for ( unsigned int i = 0; i < strlen( commandString ); i++ ) {

    // scan command string character wise & write
    singleCharacter = commandString[i];
    ssize_t bytes_written;
    bytes_written = write( fIoPortFileDescriptor, &singleCharacter, 1 );
    if ( bytes_written < 0 )
    {
      std::cerr << "Problem in writing to ComHandler!" << std::endl;
    }
  }

  if (sendfeed) {
    // send feed characters
    SendFeedString();
  }
}

//! Read a string from device.
/*!
\par receiveString:
  <br><b>Buffer to read into. Will be 0 terminated. receiveString array must be at least 1024 bytes long.</b>
*/
void ComHandler::ReceiveString( char *receiveString ) {
  timeval timeout;
  fd_set set;
  
  receiveString[0] = 0;
  
  FD_ZERO(&set);
  FD_SET(fIoPortFileDescriptor, &set);
  timeout.tv_sec = 1; // 1 second timeout
  timeout.tv_usec = 0;
  
  int rv = select(fIoPortFileDescriptor + 1, &set, NULL, NULL, &timeout);
  if (rv == -1) {
    std::cerr << "Error occured when reading from " << fIoPort << std::endl;
    return;
  } else if (rv == 0) {
    std::cerr << "Timeout reached when reading from " << fIoPort << std::endl;
    return;
  } else {
    int len = read(fIoPortFileDescriptor, receiveString, 1023);
    if (len == -1) {
      std::cerr << "Error occured when reading (read call) from " << fIoPort << std::endl;
      return;
    }
    receiveString[len] = 0;
    std::cout << "Read " << len << " bytes from " << fIoPort << ": " << receiveString << std::endl;
  }
  
}

//! Open I/O port.
/*!
  \internal
*/
void ComHandler::OpenIoPort( void ) {

  // open io port ( read/write | no term control | no DCD line check )
  fIoPortFileDescriptor = open( fIoPort, O_RDWR | O_NOCTTY  | O_NDELAY );

  // check if successful
  if ( fIoPortFileDescriptor == -1 ) {
    std::cerr << "[ComHandler::OpenIoPort] ** ERROR: could not open device file "
              << fIoPort << "." << std::endl;
    std::cerr << "                           (probably it's not user-writable)."
              << std::endl;
    throw BurnInException("Could not open device file");
    
  } else {
    // configure port with no delay
    fcntl( fIoPortFileDescriptor, F_SETFL, FNDELAY );
  }
}

//! Initialize I/O port.
/*!
  \internal
*/
void ComHandler::InitializeIoPort( speed_t baudrate ) {

#ifndef USE_FAKEIO

  // get and save current ioport settings for later restoring
  tcgetattr( fIoPortFileDescriptor, &fCurrentTermios );

  // CONFIGURE NEW SETTINGS

  // clear new settings struct
  bzero( &fThisTermios, sizeof( fThisTermios ) );

  // all these settings copied from stty output..

  // baud rate
  cfsetispeed( &fThisTermios, baudrate );  // input speed
  cfsetospeed( &fThisTermios, baudrate );  // output speed

  // various settings, 8N1 (no parity, 1 stopbit)
  fThisTermios.c_cflag   &= ~PARENB;
  fThisTermios.c_cflag   &= ~PARODD;
  fThisTermios.c_cflag   |=  CS8;
  fThisTermios.c_cflag   |=  HUPCL;
  fThisTermios.c_cflag   &= ~CSTOPB;
  fThisTermios.c_cflag   |=  CREAD;
  fThisTermios.c_cflag   |=  CLOCAL;
  fThisTermios.c_cflag   &= ~CRTSCTS;

  fThisTermios.c_lflag   |=  ISIG;
  fThisTermios.c_lflag   |=  ICANON;
  fThisTermios.c_lflag   &= ~ECHO;
  fThisTermios.c_lflag   |=  ECHOE;
  fThisTermios.c_lflag   |=  ECHOK;
  fThisTermios.c_lflag   &= ~ECHONL;
  fThisTermios.c_lflag   |=  IEXTEN;

  fThisTermios.c_iflag   &= ~IGNBRK;
  fThisTermios.c_iflag   &= ~BRKINT;
  fThisTermios.c_iflag   &= ~IGNPAR;
  fThisTermios.c_iflag   &= ~PARMRK;
  fThisTermios.c_iflag   &= ~INPCK;

  // right i/o/l flags ??
  fThisTermios.c_iflag   &= ~ISTRIP;
  fThisTermios.c_iflag   &= ~INLCR;
  fThisTermios.c_iflag   &= ~IGNCR;
  //  fThisTermios.c_iflag   |=  ICRNL; // DO NOT ENABLE!!
  fThisTermios.c_iflag   |=  IXON;
  fThisTermios.c_iflag   &= ~IXOFF;
  fThisTermios.c_iflag   &= ~IUCLC;
  fThisTermios.c_iflag   &= ~IXANY;
  fThisTermios.c_iflag   &= ~IMAXBEL;
  
  fThisTermios.c_iflag   &= ~IUTF8;

  // right i/o/l flags?
  fThisTermios.c_oflag   |=  OPOST;
  fThisTermios.c_oflag   &= ~OLCUC;
  fThisTermios.c_oflag   &= ~OCRNL;
  fThisTermios.c_oflag   |=  ONLCR;
  fThisTermios.c_oflag   &= ~ONOCR;
  fThisTermios.c_oflag   &= ~ONLRET;
  fThisTermios.c_oflag   &= ~OFILL;
  fThisTermios.c_oflag   &= ~OFDEL;

//   fThisTermios.c_cflag   |=  NL0;
//   fThisTermios.c_cflag   |=  CR0;
//   fThisTermios.c_cflag   |=  TAB0;
//   fThisTermios.c_cflag   |=  BS0;
//   fThisTermios.c_cflag   |=  VT0;
//   fThisTermios.c_cflag   |=  FF0;

  // commit changes
  tcsetattr( fIoPortFileDescriptor, TCSANOW, &fThisTermios );

#endif
}

//! Restore former I/O port settings.
/*!
  \internal
*/
void ComHandler::RestoreIoPort( void ) {

  // restore old com port settings
  tcsetattr( fIoPortFileDescriptor, TCSANOW, &fCurrentTermios );
}

//! Close I/O port.
/*!
  \internal
*/
void ComHandler::CloseIoPort( void ) {

  close( fIoPortFileDescriptor );
}

//! Send command termination string (<NL>).
/*!
  \internal
*/
void ComHandler::SendFeedString( void ) {

  // feed string is <NL>
  char feedString = 10;

  ssize_t bytes_written;

  bytes_written = write( fIoPortFileDescriptor, &feedString, 1 );

  if ( bytes_written < 0 )
  {
      std::cerr << "Problem in writing to ComHandler!" << std::endl;
  }
}


