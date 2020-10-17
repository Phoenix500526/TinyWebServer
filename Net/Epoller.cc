#include "Net/Epoller.h"

using namespace std;

Epoller::Epoller(int MaxEvents)
    : m_epollfd(epoll_create1(EPOLL_CLOEXEC)), m_events(MaxEvents) {
  if (m_epollfd < 0) LOG_FATAL << "FATAL Error: epoll_fd create failure";
}

Epoller::~Epoller() { close(m_epollfd); }

bool Epoller::AddFd(int fd, uint32_t events) {
  if (fd < 0) return false;
  epoll_event ev = {0};
  ev.data.fd = fd;
  ev.events = events;
  return 0 == epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::ModFd(int fd, uint32_t events) {
  if (fd < 0) return false;
  epoll_event ev = {0};
  ev.data.fd = fd;
  ev.events = events;
  return 0 == epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::DelFd(int fd) {
  if (fd < 0) return false;
  epoll_event ev = {0};
  return 0 == epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::Wait_Impl(int timeout) {
  return epoll_wait(m_epollfd, &m_events[0], static_cast<int>(m_events.size()),
                    timeout);
}

int Epoller::GetEventFd(size_t i) const {
  assert(i < m_events.size());
  return m_events[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i) const {
  assert(i < m_events.size());
  return m_events[i].events;
}