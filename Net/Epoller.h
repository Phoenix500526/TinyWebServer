#ifndef TINYWEBSERVER_NET_EPOLLER_H
#define TINYWEBSERVER_NET_EPOLLER_H

#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

#include "Logger/Logger.h"
#include "Tools/nocopyable.h"

class Poller : public nocopyable {
 public:
  virtual bool AddFd(int fd, uint32_t events) = 0;

  virtual bool ModFd(int fd, uint32_t events) = 0;

  virtual bool DelFd(int fd) = 0;

  virtual int Wait_Impl(int timeoutMs) = 0;
  int Wait(int timeoutMs = -1) { return Wait_Impl(timeoutMs); }

  virtual int GetEventFd(size_t i) const = 0;

  virtual uint32_t GetEvents(size_t i) const = 0;

  virtual ~Poller() {}
};

class Epoller : public Poller {
 public:
  explicit Epoller(int MaxEvents = 1024);
  ~Epoller() override;
  bool AddFd(int fd, uint32_t events) override;
  bool ModFd(int fd, uint32_t events) override;
  bool DelFd(int fd) override;
  int GetEventFd(size_t i) const override;
  uint32_t GetEvents(size_t i) const override;
  int Wait(int timeoutMs = -1) { return Wait_Impl(timeoutMs); }

 protected:
  int m_epollfd;
  std::vector<struct epoll_event> m_events;
  int Wait_Impl(int timeoutMs) override;
};

#endif