#include "LogFile.h"
#include "Logger.h"

#include <assert.h>
#include <inttypes.h>
#include <unistd.h>
using namespace std;

File::File(const char* filename)
    : m_fp(fopen(filename, "ae")), m_WrittenBytes(0) {
  assert(m_fp);
  setbuffer(m_fp, m_buffer, sizeof m_buffer);
}

File::File(const string& filename)
    : m_fp(fopen(filename.c_str(), "ae")), m_WrittenBytes(0) {
  assert(m_fp);
  setbuffer(m_fp, m_buffer, sizeof m_buffer);
}

File::~File() { fclose(m_fp); }

void File::flush() { fflush(m_fp); }

size_t File::write(const char* logline, size_t len) {
  return fwrite_unlocked(logline, 1, len, m_fp);
}

void File::append(const char* logline, size_t len) {
  size_t n = write(logline, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = write(logline + n, remain);
    if (x == 0) {
      int err = ferror(m_fp);
      if (err) {
        fprintf(stderr, "File::append() failed %s\n", strerror_tl(err));
      }
      break;
    }
    n += x;
    remain = len - n;
  }
  m_WrittenBytes += len;
}

const string hostname() {
  char buf[256];
  if (gethostname(buf, sizeof buf) == 0) {
    buf[strlen(buf) - 1] = '\0';
    return buf;
  }
  return "unknownhost";
}

LogFile::LogFile(const string& basename, off_t rollsize, int checkEveryN)
    : m_basename(basename),
      m_RollSize(rollsize),
      m_checkEveryN(checkEveryN),
      m_count(0),
      m_StartOfPeriod(0),
      m_LastRoll(0) {
  assert(basename.find('/') == string::npos);
  rollFile();
}

LogFile::~LogFile() = default;

string LogFile::getLogFileName(const string& basename, time_t* now) {
  string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;
  char timebuf[32];
  struct tm tm_time;
  *now = time(NULL);
  localtime_r(now, &tm_time);
  snprintf(timebuf, sizeof timebuf, ".%04d%02d%02d-%02d%02d%02d.",
           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  filename += timebuf;
  filename += hostname();
  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, ".%d", getpid());
  filename += pidbuf;
  filename += ".log";
  return filename;
}

bool LogFile::rollFile() {
  time_t now = 0;
  string filename = getLogFileName(m_basename, &now);
  time_t start = now / m_kRollPerSeconds * m_kRollPerSeconds;
  if (now > m_LastRoll) {
    m_LastRoll = now;
    m_StartOfPeriod = start;
    m_file.reset(new File(filename));
    return true;
  }
  return false;
}

void LogFile::flush() { m_file->flush(); }

void LogFile::append(const char* logline, size_t len) {
  m_file->append(logline, len);
  if (m_file->getWrittenBytes() > m_RollSize) {
    rollFile();
  } else {
    ++m_count;
    if (m_count >= m_checkEveryN) {
      m_count = 0;
      time_t now = time(NULL);
      time_t thisPeriod = now / m_kRollPerSeconds * m_kRollPerSeconds;
      if (thisPeriod != m_StartOfPeriod) {
        rollFile();
      }
    }
  }
}
