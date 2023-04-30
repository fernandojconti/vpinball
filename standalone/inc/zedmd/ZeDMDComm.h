#pragma once

#include <thread>
#include <queue>
#include <mutex>

#pragma push_macro("_WIN64")
#undef _WIN64
#include "serialib.h"
#pragma pop_macro("_WIN64")

typedef enum {
    Handshake = 0x0C,
    Compression = 0x0E,
    Chunk = 0x0D,
    ResetPalette = 0x06,
    DebugEnable = 0x63,
    RGB24 = 0x03,
    ColGray6 = 0x0B,
    Gray4 = 0x07,
    ColGray4 = 0x09,
    Gray2 = 0x08
} ZEDMD_COMMAND;

struct ZeDMDFrame {
   int size;
   UINT8* data;
};

class ZeDMDComm
{
public:
   static const int SERIAL_TIMEOUT = 100;
   static const int MAX_SERIAL_WRITE_AT_ONCE = 256;
   static const int FRAME_BUFFER_SIZE = 15;

   static const int CTRL_CHARS_HEADER_SIZE = 6;
   static constexpr UINT8 CTRL_CHARS_HEADER[] = { 0x5a, 0x65, 0x64, 0x72, 0x75, 0x6d };

public:
   ZeDMDComm();
   ~ZeDMDComm();

   bool Connect();
   void Disconnect();
   
   void Run(); 

   void QueueCommand(char command);
   void QueueCommand(char command, UINT8* buffer, int size);

private:
   bool StreamBytes(ZeDMDFrame* pFrame);

   serialib m_seriallib;
   
   std::queue<ZeDMDFrame> m_frames;
   std::thread* m_pThread;
   std::mutex m_mutex;

   int m_width;
   int m_height;
};