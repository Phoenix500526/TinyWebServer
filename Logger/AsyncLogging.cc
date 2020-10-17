#include "AsyncLogging.h"
#include <assert.h>
#include <inttypes.h>
#include <chrono>
#include "LogFile.h"

using namespace std;
AsyncLogging::AsyncLogging(const string& basename, off_t rollSize,
                           int flushInterval)
    : m_basename(basename),
      m_rollSize(rollSize),
      m_flushInterval(flushInterval),
      m_running(false),
      m_latch(1),
      m_mutex(),
      m_cond(),
      m_curBuffer(new Buffer),
      m_nxtBuffer(new Buffer),
      m_buffers() {
  m_curBuffer->bzero();
  m_nxtBuffer->bzero();
  m_buffers.reserve(16);
}

void AsyncLogging::append(const char* logline, size_t len) {
  UniqueLock lck(m_mutex);
  if (static_cast<size_t>(m_curBuffer->remainSpace()) > len) {
    m_curBuffer->append(logline, len);
  } else {
    m_buffers.push_back(std::move(m_curBuffer));
    if (m_nxtBuffer) {
      m_curBuffer = std::move(m_nxtBuffer);
    } else {
      m_curBuffer.reset(new Buffer);
    }
    m_curBuffer->append(logline, len);
    m_cond.notify_one();
  }
}

void AsyncLogging::threadFunc() {
  assert(m_running == true);
  m_latch.countDown();
  LogFile output(m_basename, m_rollSize);
  BufferPtr newBuffer1(new Buffer);
  BufferPtr newBuffer2(new Buffer);
  newBuffer1->bzero();
  newBuffer2->bzero();
  BufferPtrVec bufferToWrite;
  bufferToWrite.reserve(16);
  while (m_running) {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(bufferToWrite.empty());
    {
      UniqueLock lock(m_mutex);
      if (m_buffers.empty()) {
        //等待 flushInterval 时间后刷新
        m_cond.wait_for(lock, std::chrono::seconds(m_flushInterval));
      }
      m_buffers.push_back(std::move(m_curBuffer));
      m_curBuffer = std::move(newBuffer1);
      bufferToWrite.swap(m_buffers);
      if (!m_nxtBuffer) {
        m_nxtBuffer = std::move(newBuffer2);
      }
    }
    assert(!bufferToWrite.empty());
    if (bufferToWrite.size() > 25) {
      //错误处理部分
      char buf[256];
      chrono::system_clock::time_point now_point = chrono::system_clock::now();
      time_t now = chrono::system_clock::to_time_t(now_point);
      struct tm tm_now;
      localtime_r(&now, &tm_now);
      snprintf(buf, sizeof buf,
               "Dropped log message at %04d-%02d-%02d %02d:%02d:%02d, "
               "%zd large buffers\n",
               tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
               tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec,
               bufferToWrite.size() - 2);
      fputs(buf, stderr);
      output.append(buf, strlen(buf));
      bufferToWrite.erase(bufferToWrite.begin() + 2, bufferToWrite.end());
    }
    for (const auto& buf : bufferToWrite) {
      output.append(buf->data(), buf->length());
    }
    if (bufferToWrite.size() > 2) {
      bufferToWrite.resize(2);
    }
    if (!newBuffer1) {
      assert(!bufferToWrite.empty());
      newBuffer1 = std::move(bufferToWrite.back());
      bufferToWrite.pop_back();
      newBuffer1->reset();
    }
    if (!newBuffer2) {
      assert(!bufferToWrite.empty());
      newBuffer2 = std::move(bufferToWrite.back());
      bufferToWrite.pop_back();
      newBuffer2->reset();
    }
    bufferToWrite.clear();
    output.flush();
  }
  output.flush();
}