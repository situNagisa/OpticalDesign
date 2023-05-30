#pragma once

#include "NSL/io/MultiplexIO.h"
#include "NSL/LinuxHeader.h"
#include "NSL/file/File.h"

//”√”⁄≤‚ ‘
#if NGS_PLATFORM == NGS_WINDOWS
NGS_C struct pollfd
{
	int fd;			/* File descriptor to poll.  */
	short int events;		/* Types of events poller cares about.  */
	short int revents;		/* Types of events that actually occurred.  */
};

extern int poll(struct pollfd* __fds, nfds_t __nfds, int __timeout);
#endif

nsl::MultIO::Tag::operator ngs::type::int16()const {
	ngs::int16 flag = 0;
	if (in)flag |= POLLIN;
	if (read_norm)flag |= POLLRDNORM;
	if (read_band)flag |= POLLRDBAND;
	if (pri)flag |= POLLPRI;
	if (read_hup)flag |= POLLRDHUP;
	if (out)flag |= POLLOUT;
	if (write_norm)flag |= POLLWRNORM;
	if (write_band)flag |= POLLWRBAND;
	if (error)flag |= POLLERR;
	if (hup)flag |= POLLHUP;
	if (no_val)flag |= POLLNVAL;
	if (message)flag |= POLLMSG;

	return flag;
}
nsl::MultIO::Tag& nsl::MultIO::Tag::operator=(ngs::int16 flag) {
	Clear();
	if (ngs::Bits(flag, POLLIN))in = true;
	if (ngs::Bits(flag, POLLRDNORM))read_norm = true;
	if (ngs::Bits(flag, POLLRDBAND))read_band = true;
	if (ngs::Bits(flag, POLLPRI))pri = true;
	if (ngs::Bits(flag, POLLRDHUP))read_hup = true;
	if (ngs::Bits(flag, POLLOUT))out = true;
	if (ngs::Bits(flag, POLLWRNORM))write_norm = true;
	if (ngs::Bits(flag, POLLWRBAND))write_band = true;
	if (ngs::Bits(flag, POLLERR))error = true;
	if (ngs::Bits(flag, POLLHUP))hup = true;
	if (ngs::Bits(flag, POLLNVAL))no_val = true;
	if (ngs::Bits(flag, POLLMSG))message = true;
}

int nsl::MultIO::Poll(int timeout) {
	using p = pollfd;
	std::vector<p> polls = {};
	for (auto& i : _files) {
		auto [file, event, revents] = i;
		polls.emplace_back(file->_fd, ngs::int16(event), ngs::int16(revents));
	}
	int ret = poll(polls.data(), polls.size(), timeout);
	for (size_t i = 0; i < polls.size(); i++)
	{
		auto [file, event, revents] = _files.at(i);
		revents = polls.at(i).revents;
	}
	return ret;
}
