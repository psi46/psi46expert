#include <libusb-1.0/libusb.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <time.h> // needed for usleep function

#include "USBInterface.h"

// needed for threaded readout of FTDI
#include <pthread.h> 
#include <semaphore.h>

static struct ftdi_context ftdic;

// the read buffer needs to be accessable outside of our USB class
#define BUFSIZE 0x200000
static pthread_t readerthread;
static sem_t buf_data, buf_space;
static unsigned char read_buffer[BUFSIZE];
static int32_t head, tail; // read buffer is used as ring buffer

using namespace std;

static void add_to_buf (unsigned char c) {
    int32_t nh;

    sem_wait (&buf_space);
    if (head == (BUFSIZE -1)) nh = 0;
    else                      nh = head + 1;

    if (nh == tail) {
        fprintf (stderr, "USBInterface: threaded read routine: overflow of circular buffer. Cannot happen!\n");
        exit (1);
    }
    read_buffer[head] = c;
    head = nh;
    sem_post (&buf_data);
}

static void *reader (void *arg) {
  // there is no non-blocking read command implemented in libftdi ->
  // therefore we use multithreading and a static buffer to emulate
  // non-blocking calls
    struct ftdi_context *handle = (struct ftdi_context *)(arg);
    unsigned char buf[0x1000];
    int32_t br, i;

    while (1) {
        pthread_testcancel();
        br = ftdi_read_data (handle, buf, sizeof(buf));
	if (br< 0){
	  std::cout << " ERROR during USB read polling: error code from libusb_bulk_transfer(): " << br << std::endl;
	}
	if (br > 0){
	  for (i=0; i<br; i++){
	    add_to_buf (buf[i]);
	  }
	}
    }
    return NULL;
}


CUSB::CUSB(){
      m_posR = m_sizeR = m_posW = 0;
      m_timeout = 15000; // maximum time to wait for read call in ms
      isUSB_open = false;
      ftdiStatus = 0;
      enumPos = enumCount = 0;
      ftdiStatus = ftdi_init(&ftdic);
      productID =  0x6014; // init with new TB usb chip product id (FT232H)
      vendorID = 0x0403; // Future Technology Devices International, Ltd
      if ( ftdiStatus < 0)
	{
	  cout <<  "USBInterface constructor: ftdi_init failed" << endl;
	  exit(1);
	}
}

CUSB::~CUSB(){ 
    pthread_cancel(readerthread);
    pthread_join(readerthread, NULL);
    Close(); 
    ftdi_deinit(&ftdic);
  }

const char* CUSB::GetErrorMsg()
{
  ftdi_get_error_string(&ftdic);
}


bool CUSB::EnumFirst(uint32_t &nDevices)
{
  struct ftdi_device_list *  	devlist;

  ftdiStatus =  ftdi_usb_find_all(&ftdic, &devlist,vendorID,productID);
  if( ftdiStatus != 0) {
    nDevices = enumCount = enumPos = 0;
    return false;
  }
  ftdi_list_free(&devlist);
  enumCount = ftdiStatus;
  nDevices = ftdiStatus;
  enumPos = 0;
  return true;
}


bool CUSB::EnumNext(char name[])
{
  if( isUSB_open) { 
    std::cout << " Warning: Trying to call USBInterface::EnumNext() while other USB device still open" << std::endl;
    return false; 
  }

  struct ftdi_device_list *  	devlist;

  ftdiStatus =  ftdi_usb_find_all(&ftdic, &devlist,vendorID,productID);
  if( ftdiStatus != 0) {
    enumCount = enumPos = 0;
    return false;
  }
  enumCount = ftdiStatus;
  if( enumPos >= enumCount) return false;

  // go to the position of the enumPos pointer
  for (int32_t i=0; i<enumPos; i++) devlist->next;
  
  char manufacturer[128], description[128], serial[128];

  if ((ftdiStatus = ftdi_usb_get_strings(&ftdic,devlist->dev, manufacturer, 128, description, 128, serial, 128)) < 0)
    {
      std::cout << " USBInterface::EnumNext(): Error polling USB device number " << enumPos << std::endl;
      return EXIT_FAILURE;
    }
  ftdi_list_free(&devlist);
  enumPos++;
  return true;
}


bool CUSB::Open(char serialNumber[])
{
  if( isUSB_open) { 
    std::cout << " Warning: Trying to open new USB device while other device still open" << std::endl;
    return false; 
  }

  m_posR = m_sizeR = m_posW = 0;
  ftdiStatus = ftdi_usb_open_desc(&ftdic, vendorID, productID, NULL, serialNumber);
  if( ftdiStatus < 0) {
    std::cout << " Warning: FTDI returned status code " << ftdiStatus << ", will try to detach ftdi_sio and usbserial kernel modules " << std::endl;

    /* maybe the ftdi_sio and usbserial kernel modules are attached to the device */
    /* try to detach them using the libusb library directly */

    /* prepare libusb structures */
    libusb_device ** list;
    libusb_device_handle *handle;
    struct libusb_device_descriptor descriptor;

    /* initialise libusb and get device list*/
    libusb_init(NULL);
    ssize_t ndevices = libusb_get_device_list(NULL, &list);
    if( ndevices < 0)
      return false;

    char serial [20];

    bool found = false;

    /* loop over all USB devices */
    for( int32_t dev = 0; dev < ndevices; dev++) {
      /* get the device descriptor */
      int32_t ok = libusb_get_device_descriptor(list[dev], &descriptor);
      if( ok != 0)
        continue;

      /* we're only interested in devices with one vendor and product ID */
      if( descriptor.idVendor != vendorID || descriptor.idProduct != productID)
        continue;

      /* open the device */
      ok = libusb_open(list[dev], &handle);
      if( ok != 0)
        continue;

      /* Read the serial number from the device */
      ok = libusb_get_string_descriptor_ascii(handle, descriptor.iSerialNumber, (unsigned char *) serial, 20);
      if( ok < 0)
        continue;

      /* Check the device serial number */
      if( strcmp(serialNumber, serial) == 0) {
        /* that's our device */
        found = true;

        /* Detach the kernel module from the device */
        ok = libusb_detach_kernel_driver(handle, 0);
        if( ok == 0)
          printf("Detached kernel driver from selected testboard.\n");
        else
          printf("Unable to detach kernel driver from selected testboard.\n");
        break;
      }

      libusb_close(handle);
    }

    libusb_free_device_list(list, 1);

    /* if the device was not found in the previous loop, don't try again */
    if( !found)
      return false;

    /* try to re-open with the detached device */
    ftdiStatus = ftdi_usb_open_desc(&ftdic, vendorID, productID, NULL, serialNumber);
    if( ftdiStatus < 0)
      return false;
  } else {
    std::cout << " FTDI successfully opened connection to device " << std::endl;
  }

  std::cout << " resetting mode for FTDI chip " << std::endl;
  int32_t status =  ftdi_set_bitmode(&ftdic, 0xFF, 0x40);
  if (status < 0){
    std::cout << " ERROR issuing reset: return code " << status << std::endl;
  }
  usleep(10000); // wait 10 ms
  std::cout << " setting bit mode for FTDI chip " << std::endl;
  status =  ftdi_set_bitmode(&ftdic, 0xFF, 0x40);
  if (status < 0){
    std::cout << " ERROR setting bit mode: return code " << status << std::endl;
  }


  sem_init (&buf_data, 0, 0);
  sem_init (&buf_space, 0, BUFSIZE);
  pthread_create (&readerthread, NULL, reader, &ftdic);

  isUSB_open = true;
  return true;
}


void CUSB::Close()
{
  if( !isUSB_open) return;
  ftdi_usb_close(&ftdic);
  isUSB_open = 0;
}

bool CUSB::WriteCommand(unsigned char x){
  const unsigned char CommandChar = ESC_EXTENDED; 
  bool StatusCmdBit = Write(sizeof(char), &CommandChar); // ESC_EXTENDED 
  return Write(sizeof(char),&x) && StatusCmdBit;
}

bool CUSB::Write(uint32_t bytesToWrite, const void *buffer)
{
  if( !isUSB_open) return false;
  uint32_t k=0;
  for( k=0; k < bytesToWrite; k++ ) {
    if( m_posW >= USBWRITEBUFFERSIZE) { if( !Flush()) return false; }
    m_bufferW[m_posW++] = ((unsigned char*)buffer)[k];
  }
  return true;
}


bool CUSB::Flush()
{
  int32_t bytesToWrite = m_posW;
  m_posW = 0;

  if( !isUSB_open) return false;

  if( !bytesToWrite) return true;

  ftdiStatus = ftdi_write_data(&ftdic, m_bufferW, bytesToWrite);

  if( ftdiStatus < 0) return false;
  if( ftdiStatus != bytesToWrite) { 
    std::cout<< " Warning: USBInterface: mismatch of bytes sent to USB chip and bytes written! " << std::endl;
    return false; 
  }

  return true;
}

bool CUSB::FillBuffer(uint32_t minBytesToRead)
{
  cout << " USBInterface: FillBuffer() called but this function is not implemted anymore for libftdi " << endl;
  return true;
}


bool CUSB::Read(uint32_t bytesToRead, void *buffer, uint32_t &bytesRead)
{
    // Copy over data from the circular buffer
    int32_t i;
    int32_t timewasted = 0; // time in ms wasted in this routine

      for (i=0; i<bytesToRead; i++) {
	bool bufferready = true;
	if (tail == head) bufferready = false;
	if (!bufferready){
	  while (!bufferready && timewasted<m_timeout){
	    if (timewasted==(m_timeout/10)) cout<< "USBInterface: Read(): data not ready after " << timewasted << "ms yet! Will wait for up to " << m_timeout << "ms"<< flush;
	    if (timewasted>m_timeout && timewasted%100==0 ) cout << "." << flush;
	    usleep(1000); //wait 1 ms
	    timewasted++;
	    if (tail != head) bufferready = true;	    
	  }
	  // if timeout message was printed before show the conclusion now
	  if (timewasted >= (m_timeout/10)){
	    if (bufferready) cout << "..done!" << endl;
	    else cout << "..failed! :(    .. maybe adjust timeout setting (method SetTimeout(int)) for this call?" <<endl;
	  }
	}
	if (bufferready){
	  sem_wait (&buf_data);	
	  ((unsigned char*)buffer)[i] = read_buffer[tail];
	  if (tail == (BUFSIZE -1)) tail = 0;
	  else                      tail++;
	  sem_post (&buf_space);
	} 
	else // buffer was not ready and reading it timed out so we stop attempting it now
	  break;
      }
      bytesRead = i;
      return (bytesRead==bytesToRead);
}

//----------------------------------------------------------------------
bool CUSB::Clear()
{
  if( !isUSB_open) return false;

  ftdiStatus = ftdi_usb_purge_buffers(&ftdic);

  // drain our buffer.
  while (head != tail) {
    unsigned char t;
    uint32_t n;
    Read(1, &t, n);
  }

  m_posR = m_sizeR = 0;
  m_posW = 0;

  return ftdiStatus != 0;
}

//----------------------------------------------------------------------
bool CUSB::Show()
{
  cout << " USB status: " << endl;
  if( !isUSB_open) {
    cout << "  - USB connection not open " << endl;
    return false;
  }
  std::cout << std::hex << "  - looking for test boards with product ID " << productID << " and vendor ID " << vendorID << std::dec << std::endl; 
  cout << "  - max timeout for read calls set to " << m_timeout << "ms" << endl;

  unsigned char latency;
  if (ftdi_get_latency_timer(&ftdic,&latency)==0)  cout << "  - FTDI latency timer set to " << (int) latency << endl;
  cout << "  - data waiting in local read buffer: " << !(tail == head) << endl;
  

  
  return true;
}

//----------------------------------------------------------------------
int32_t CUSB::GetQueue()
{
  cout << " USBInterface: GetQueue() called but this function is not implemted for libftdi " << endl;
  return 0;
}

bool CUSB::WaitForFilledQueue(int32_t pSize,int32_t pMaxWait)
{
  // this function has no purpose when using libftdi: we implement our own read buffer and poll 
  if( !isUSB_open ) return false;
  return true;
}

//----------------------------------------------------------------------
bool CUSB::Read_String(char *s, uint16_t maxlength)
{
  char ch = 0;
  uint16_t i=0;
  do {
    if( !Read_CHAR(ch)) return false;
    if( i<maxlength) { s[i] = ch; i++; }
  }
  while (ch != 0);
  if( i >= maxlength) s[maxlength-1] = 0;
  return true;
}


bool CUSB::Write_String(const char *s)
{
  do {
    if( !Write_CHAR(*s) ) return false;
    s++;
  }
  while (*s != 0);
  return true;
}
