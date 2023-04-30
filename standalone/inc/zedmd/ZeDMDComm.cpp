#include "stdafx.h"
#include "ZeDMDComm.h"
#include "inc/miniz/miniz.h"

/**
 * Derived from https://github.com/freezy/dmd-extensions/blob/master/LibDmd/Output/ZeDMD/ZeDMDComm.cs
 * and https://github.com/PPUC/pinmame/blob/threads/src/ppuc/zedmd/zedmd.cpp
 */

ZeDMDComm::ZeDMDComm()
{
   m_width = 0;
   m_height = 0;
}

ZeDMDComm::~ZeDMDComm()
{
   if (m_pThread) {
      m_pThread->join();
      delete m_pThread;
   }
}

bool ZeDMDComm::Connect()
{
   m_width = 0;
   m_height = 0;

   char szDevice[23];
#if (defined(__linux) || defined(__linux__))
   sprintf_s(szDevice, sizeof(szDevice), "/dev/ttyUSB%d", 1);
#elif defined(__APPLE__)
   sprintf_s(szDevice, sizeof(szDevice), "/dev/cu.usbserial-%04d", 1);
#endif

   if (m_seriallib.openDevice(szDevice, 921600, SERIAL_DATABITS_8, SERIAL_PARITY_NONE, SERIAL_STOPBITS_1) != 1)
      return false;
   
   m_seriallib.clearDTR();
   m_seriallib.setRTS();
   std::this_thread::sleep_for(std::chrono::milliseconds(200));
   m_seriallib.clearRTS();
   m_seriallib.clearDTR();
   std::this_thread::sleep_for(std::chrono::milliseconds(200));

   UINT8 data[8] = { 0 };

   while (m_seriallib.available() > 0)
      m_seriallib.readBytes(data, 8, 200); 

   m_seriallib.writeBytes(CTRL_CHARS_HEADER, CTRL_CHARS_HEADER_SIZE);
   m_seriallib.writeChar(ZEDMD_COMMAND::Handshake);

   if (m_seriallib.readBytes(data, 8, 1000)) {
      if (!memcmp(data, CTRL_CHARS_HEADER, 4)) {
         int width = data[4] + data[5] * 256;
         int height = data[6] + data[7] * 256;

         if (width > 0 && height > 0) {
            UINT8 response;

            if (!(m_seriallib.readChar((char*)&response, SERIAL_TIMEOUT) && response == 'R')) {
               Disconnect();
               return false;
            }

            m_seriallib.writeBytes(CTRL_CHARS_HEADER, 6);  // Enable compression
            m_seriallib.writeChar(ZEDMD_COMMAND::Compression);
              
            if (!(m_seriallib.readChar((char*)&response, SERIAL_TIMEOUT) && response == 'A')) {
               Disconnect();
               return false;
            }

            if (!(m_seriallib.readChar((char*)&response, SERIAL_TIMEOUT) && response == 'R')) {
               Disconnect();
               return false;
            }

            m_seriallib.writeBytes(CTRL_CHARS_HEADER, 6);  // Set chunk size
            m_seriallib.writeChar(ZEDMD_COMMAND::Chunk);
            m_seriallib.writeChar(MAX_SERIAL_WRITE_AT_ONCE / 256);
                 
            if (!(m_seriallib.readChar((char*)&response, SERIAL_TIMEOUT) && response == 'A')) {
               Disconnect();
               return false;
            }

            if (!(m_seriallib.readChar((char*)&response, SERIAL_TIMEOUT) && response == 'R')) {
               Disconnect();
               return false;
            }

            PLOGI.printf("ZeDMD found: device=%s, width=%d, height=%d", szDevice, width, height);     

            QueueCommand(ZEDMD_COMMAND::ResetPalette);

            m_width = width;
            m_height = height;
            
            return true;
         }
      }
   }

   Disconnect();
   return false;
}

void ZeDMDComm::QueueCommand(char command)
{
   ZeDMDFrame frame;
   frame.data = (UINT8*)malloc(CTRL_CHARS_HEADER_SIZE + 1);
   memcpy(frame.data, CTRL_CHARS_HEADER, CTRL_CHARS_HEADER_SIZE);
   frame.data[CTRL_CHARS_HEADER_SIZE] = command;
   frame.size = CTRL_CHARS_HEADER_SIZE + 1;
   m_frames.push(frame);
}

void ZeDMDComm::QueueCommand(char command, UINT8* data, int size) 
{  
   if (!m_seriallib.isDeviceOpen())
      return;

   if (data && size > 0) {
      UINT8* buffer = (UINT8*)malloc(size);
      memcpy(buffer, data, size);
      new std::thread([this, command, buffer, size](){
          ZeDMDFrame frame;
          mz_ulong compressedSize = mz_compressBound(size);
          frame.data = (UINT8*)malloc(CTRL_CHARS_HEADER_SIZE + 3 + compressedSize);
          memcpy(frame.data, CTRL_CHARS_HEADER, CTRL_CHARS_HEADER_SIZE);
          frame.data[CTRL_CHARS_HEADER_SIZE] = command;
          mz_compress(frame.data + CTRL_CHARS_HEADER_SIZE + 3, &compressedSize, buffer, size);
          frame.data[CTRL_CHARS_HEADER_SIZE + 1] = (UINT8)(compressedSize >> 8 & 0xFF);
          frame.data[CTRL_CHARS_HEADER_SIZE + 2] = (UINT8)(compressedSize & 0xFF);
          frame.size = CTRL_CHARS_HEADER_SIZE + 3 + compressedSize;
          m_frames.push(frame);
          free(buffer);
      });
   }
}

void ZeDMDComm::Disconnect()
{
   if (m_seriallib.isDeviceOpen())
      m_seriallib.closeDevice();
}

void ZeDMDComm::Run()
{
   PLOGI.printf("ZeDMD run thread starting");

   m_pThread = new std::thread([this](){ 
      while (m_seriallib.isDeviceOpen()) {
         if (m_frames.size() >= FRAME_BUFFER_SIZE) {
              while(m_frames.size() > 2) {
                 ZeDMDFrame frame = m_frames.front();
                 free(frame.data);
                 m_frames.pop();
              }  
         }
         
         if (!m_frames.empty()) {
            ZeDMDFrame frame = m_frames.front();
            StreamBytes(&frame);
            free(frame.data);
            m_frames.pop();
         }
         else
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
      }

      PLOGI.printf("ZeDMD run thread finished");
   });
}

bool ZeDMDComm::StreamBytes(ZeDMDFrame* pFrame)
{     
  bool success = true;
  int position = 0;
  UINT8 response = 0;
  while (position < pFrame->size && success) {
     int chunkSize = ((pFrame->size - position) < MAX_SERIAL_WRITE_AT_ONCE) ? (pFrame->size - position) : MAX_SERIAL_WRITE_AT_ONCE;
     m_seriallib.writeBytes(pFrame->data + position, chunkSize);
     position += chunkSize;

     if (m_seriallib.readChar((char*)&response, SERIAL_TIMEOUT))
        if (response == 'E')
           success = false;
  }

  if (m_seriallib.readChar((char*)&response, SERIAL_TIMEOUT))
     if (response == 'E') {
        success = false;
        m_seriallib.readChar((char*)&response, SERIAL_TIMEOUT);
     }

   return success;
}