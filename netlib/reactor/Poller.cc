#include "Poller.h"
#include "Channel.h"
#include <poll.h>
#include <cassert>
using namespace mylib;

Poller::Poller(EventLoop *loop) : ownerLoop_(loop) {}
Poller::~Poller() {}

Timestamp Poller::poll(int timeoutMs, ChannelList *activeChannels)
{
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0)
    {
        LOG_TRACE("%d events happended", numEvents);
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (numEvents == 0)
    {
        LOG_TRACE("nothing happended");
    }
    else
    {
        LOG_SYSERR("Poller::poll()");
    }
    return now;
}

void Poller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; pfd++)
    {
        if (pfd->revents > 0)
        {
            numEvents--;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel *channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel *channel)
{
    assertInLoopThread();
    LOG_TRACE("fd = %d events = %d", channel->fd(), channel->events());
    if (channel->index() < 0)
    {
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    }
    else
    {
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent())
        {
            pfd.fd = -1;
        }
    }
}

void Poller::removeChannel(Channel *channel)
{
    assertInLoopThread();
    LOG_TRACE("fd = %d", channel->fd());
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));

    const struct pollfd &pfd = pollfds_[idx];
    channels_.erase(channel->fd());

    if (static_cast<size_t>(idx) == pollfds_.size() - 1)
    {
        pollfds_.pop_back();
    }
    else
    {
        int channelAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channelAtEnd < 0)
        {
            channelAtEnd = -channelAtEnd - 1;
        }
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }
}