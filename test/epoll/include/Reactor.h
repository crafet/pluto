
#ifndef REACTOR_H_
#define REACTOR_H_

#include <sys/time.h>
#include <cassert>
#include <cerrno>
#include <tr1/functional>
#include <string>
#include <vector>
#include "ev.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

namespace ads
{
namespace net
{

using std::tr1::function;
using namespace std::tr1::placeholders;

typedef ev_tstamp tstamp;

class Reactor
{
public:

	enum
	{
		UNDEF = EV_UNDEF,
		NONE = EV_NONE,
		READ = EV_READ,
		WRITE = EV_WRITE,
		TIMER = EV_TIMER,
		SIGNAL = EV_SIGNAL,
		ASYNC = EV_ASYNC,
		ERROR = EV_ERROR
	};

	enum
	{
		AUTO = EVFLAG_AUTO,
		NOENV = EVFLAG_NOENV,
		FORKCHECK = EVFLAG_FORKCHECK,
		SELECT = EVBACKEND_SELECT,
		POLL = EVBACKEND_POLL,
		EPOLL = EVBACKEND_EPOLL,
		KQUEUE = EVBACKEND_KQUEUE,
		DEVPOLL = EVBACKEND_DEVPOLL,
		PORT = EVBACKEND_PORT
	};

	enum
	{
		NOWAIT = EVRUN_NOWAIT, ONCE = EVRUN_ONCE
	};

	Reactor(std::string const& name, unsigned int flags = EVFLAG_AUTO);

	~Reactor();

	void run(int flags = 0);

	void stop();

	tstamp now();

	struct ev_loop * loop();

	char const* backend() const;

private:

	Reactor();

	Reactor(Reactor const&);

	Reactor & operator=(Reactor const&);

	std::string m_name;

	struct ev_loop * m_loop;
};

template<typename W>
class Watcher: protected W
{
public:

	typedef function<void(int revents)> Handler;

	Watcher(Reactor & r, Handler h);

	Watcher(Reactor & r);

	Reactor const& reactor() const;

	Reactor & reactor();

	void operator()(int revents);

	bool is_active() const;

	bool is_pending() const;

protected:

	static void
	forward_callback(struct ev_loop *loop, W *w, int revents);

	struct ev_loop * loop();

	void set(Handler handler);

private:

	Watcher();

	Watcher(Watcher const&);

	Watcher & operator=(Watcher const&);

	Reactor & m_reactor;

	Handler m_handler;
};

class IOWatcher: public Watcher<ev_io>
{
public:

	typedef Watcher<ev_io> Base;

	typedef function<void(IOWatcher *watcher, int revents)> Handler;

	IOWatcher(Reactor & r, Handler h, int fd = -1, int events = Reactor::READ);

	IOWatcher(Reactor & r);

	~IOWatcher();

	void start();

	void stop();

	int fd() const;

	int events() const;

	void set(int fd, int events);

	void set(int events);

	void set(Handler handler);

	template<typename T, typename F>
	void set(T * obj, F memfn)
	{
		set(bind(memfn, obj, _1, _2));
	}
};

class TimerWatcher: public Watcher<ev_timer>
{
public:

	typedef Watcher<ev_timer> Base;

	typedef function<void(TimerWatcher *watcher, int revents)> Handler;

	TimerWatcher(Reactor & r, Handler h, tstamp after, tstamp repeat = 0.);

	TimerWatcher(Reactor & r);

	~TimerWatcher();

	void start();

	void stop();

	void again();

	tstamp repeat() const;

	tstamp remaining();

	void set(tstamp after, tstamp repeat = 0.);

	void set(Handler handler);

	template<typename T, typename F>
	void set(T * obj, F memfn)
	{
		set(bind(memfn, obj, _1, _2));
	}
};

class SignalWatcher: public Watcher<ev_signal>
{
public:

	typedef Watcher<ev_signal> Base;

	typedef function<void(SignalWatcher *watcher, int revents)> Handler;

	SignalWatcher(Reactor & r, Handler h, int signum = 0);

	SignalWatcher(Reactor & r);

	~SignalWatcher();

	int signum() const;

	void start();

	void stop();

	void set(int signum);

	void set(Handler handler);

	template<typename T, typename F>
	void set(T * obj, F memfn)
	{
		set(bind(memfn, obj, _1, _2));
	}
};

class AsyncWatcher: public Watcher<ev_async>
{
public:

	typedef Watcher<ev_async> Base;

	typedef function<void(AsyncWatcher *watcher, int revents)> Handler;

	AsyncWatcher(Reactor & r, Handler h);

	AsyncWatcher(Reactor & r);

	~AsyncWatcher();

	void start();

	void stop();

	void set(Handler handler);

	template<typename T, typename F>
	void set(T * obj, F memfn)
	{
		set(bind(memfn, obj, _1, _2));
	}

	void send();

	bool pending();
};


typedef uint8_t byte;

class Buffer
{
public:

	static const size_t kInitialSize = 4 * 1024; // 1024;

	Buffer(size_t initial_size = kInitialSize) :
			data_(initial_size), reader_index_(0), writer_index_(0)
	{
		assert(readable() == 0);
		assert(writable() == initial_size);
		assert(prependable() == 0);
	}

	// default copy-ctor, dtor and assignment are fine

	void swap(Buffer& rhs)
	{
		std::swap(data_, rhs.data_);
		std::swap(reader_index_, rhs.reader_index_);
		std::swap(writer_index_, rhs.writer_index_);
	}

	size_t readable() const
	{
		return writer_index_ - reader_index_;
	}

	size_t writable() const
	{
		return data_.size() - writer_index_;
	}

	size_t prependable() const
	{
		return reader_index_;
	}

	byte* read_ptr()
	{
		return data_.data() + reader_index_;
	}

	byte const* read_ptr() const
	{
		return data_.data() + reader_index_;
	}

	byte* write_ptr()
	{
		return data_.data() + writer_index_;
	}

	byte const* write_ptr() const
	{
		return data_.data() + writer_index_;
	}

	void reset()
	{
		reader_index_ = 0;
		writer_index_ = 0;
	}

	size_t retrieve(void * buf, size_t len)
	{
		size_t readed = std::min(len, readable());
		memcpy(buf, read_ptr(), readed);
		retrieve(readed);
		return readed;
	}

	void retrieve(size_t len)
	{
		assert(len <= readable());
		reader_index_ += len;
		if (0 == readable())
			reset();
	}

	void append(void const* buf, size_t len)
	{
		make_space(len);
		memcpy(write_ptr(), buf, len);
		append(len);
	}

	void append(void const* buf, size_t offset, size_t size)
	{
		append(static_cast<byte const*>(buf) + offset, size - offset);
	}

	void append(size_t len)
	{
		assert(len <= writable());
		writer_index_ += len;
	}

	void make_space(size_t more)
	{
		if (writable() >= more)
			return;

		if (writable() + prependable() < more)
		{
			data_.resize(writer_index_ + more);
		}
		else
		{
			// move readable data to the front, make space inside buffer
			size_t used = readable();
			std::copy(read_ptr(), write_ptr(), data_.begin());
			reader_index_ = 0;
			writer_index_ = reader_index_ + used;
			assert(used == readable());
		}
	}

private:

	std::vector<byte> data_;

	size_t reader_index_;

	size_t writer_index_;
};

inline Reactor::Reactor(std::string const& name, unsigned int flags) :
		m_name(name), m_loop(ev_loop_new(flags))
{
}

inline Reactor::~Reactor()
{
	stop();
	if (!ev_is_default_loop(m_loop))
	{
		ev_loop_destroy(m_loop);
		m_loop = NULL;
	}
}

inline void Reactor::run(int flags)
{
	ev_run(m_loop, flags);
}

inline void Reactor::stop()
{
	ev_break(m_loop, EVBREAK_ALL);
}

inline tstamp Reactor::now()
{
	return ev_now(m_loop);
}

inline struct ev_loop * Reactor::loop()
{
	return m_loop;
}

inline char const* Reactor::backend() const
{
	switch(ev_backend(m_loop))
	{
	case EVBACKEND_SELECT:
		return "select";
	case EVBACKEND_POLL:
		return "poll";
	case EVBACKEND_EPOLL:
		return "epoll";
	case EVBACKEND_KQUEUE:
		return "kqueue";
	case EVBACKEND_DEVPOLL:
		return "devpoll";
	case EVBACKEND_PORT:
		return "port";
	default:
		return "unknown";
	}
}

template<typename W>
Watcher<W>::Watcher(Reactor & r, Handler h) :
		W(), m_reactor(r), m_handler(h)
{
	ev_init(this, forward_callback);
}

template<typename W>
Watcher<W>::Watcher(Reactor & r) :
		W(), m_reactor(r), m_handler()
{
	ev_init(this, forward_callback);
}

template<typename W>
Reactor const& Watcher<W>::reactor() const
{
	return m_reactor;
}

template<typename W>
Reactor & Watcher<W>::reactor()
{
	return m_reactor;
}

template<typename W>
void Watcher<W>::operator()(int revents)
{
	m_handler(revents);
}

template<typename W>
bool Watcher<W>::is_active() const
{
	return ev_is_active (reinterpret_cast<const ev_watcher *>(this));
}

template<typename W>
bool Watcher<W>::is_pending() const
{
	return ev_is_pending (static_cast<const ev_watcher *>(this));
}

template<typename W>
void Watcher<W>::forward_callback(struct ev_loop *loop, W *w,
		int revents)
{
	Watcher * watcher = static_cast<Watcher *>(w);
	(*watcher)(revents);
}

template<typename W>
struct ev_loop * Watcher<W>::loop()
{
	return m_reactor.loop();
}

template<typename W>
void Watcher<W>::set(Handler handler)
{
	m_handler = handler;
}

inline IOWatcher::IOWatcher(Reactor & r, Handler h, int f, int e) :
		Base(r, std::bind1st(h, this))
{
	set(f, e);
}

inline IOWatcher::IOWatcher(Reactor & r) :
		Base(r)
{
	ev_io_set(static_cast<ev_io *>(this), -1, 0);
}

inline IOWatcher::~IOWatcher()
{
	this->stop();
}

inline void IOWatcher::start()
{
	ev_io_start(loop(), this);
}

inline void IOWatcher::stop()
{
	ev_io_stop(loop(), this);
}

inline int IOWatcher::fd() const
{
	return Base::fd;
}

inline int IOWatcher::events() const
{
	return Base::events;
}

inline void IOWatcher::set(int f, int e)
{
	int isActive = is_active();

	if (isActive)
		stop();

	ev_io_set(static_cast<ev_io *>(this), f, e);

	if (isActive)
		start();
}

inline void IOWatcher::set(int e)
{
	set(Base::fd, e);
}

inline void IOWatcher::set(Handler handler)
{
	Base::set(std::bind1st(handler, this));
}

inline TimerWatcher::TimerWatcher(Reactor & r, Handler h, tstamp a, tstamp t) :
		Base(r, std::bind1st(h, this))
{
	set(a, t);
}

inline TimerWatcher::TimerWatcher(Reactor & r) :
		Base(r)
{
}

inline TimerWatcher::~TimerWatcher()
{
	this->stop();
}

inline void TimerWatcher::start()
{
	ev_timer_start(loop(), this);
}

inline void TimerWatcher::stop()
{
	ev_timer_stop(loop(), this);
}

inline tstamp TimerWatcher::repeat() const
{
	return Base::repeat;
}

inline tstamp TimerWatcher::remaining()
{
	return ev_timer_remaining(loop(), this);
}

inline void TimerWatcher::again()
{
	ev_timer_again(loop(), this);
}

inline void TimerWatcher::set(tstamp a, tstamp r)
{
	int isActive = is_active();

	if (isActive)
		stop();

	ev_timer_set(reinterpret_cast<ev_timer *>(this), a, r);

	if (isActive)
		start();
}

inline void TimerWatcher::set(Handler handler)
{
	Base::set(std::bind1st(handler, this));
}

inline SignalWatcher::SignalWatcher(Reactor & r, Handler h, int s) :
		Base(r, std::bind1st(h, this))
{
	set(s);
}

inline SignalWatcher::SignalWatcher(Reactor & r) :
		Base(r)
{
}

inline SignalWatcher::~SignalWatcher()
{
	this->stop();
}

inline void SignalWatcher::start()
{
	ev_signal_start(loop(), this);
}

inline void SignalWatcher::stop()
{
	ev_signal_stop(loop(), this);
}

inline int SignalWatcher::signum() const
{
	return Base::signum;
}

inline void SignalWatcher::set(int s)
{
	int isActive = is_active();

	if (isActive)
		stop();

	ev_signal_set(static_cast<ev_signal *>(this), s);

	if (isActive)
		start();
}

inline void SignalWatcher::set(Handler handler)
{
	Base::set(std::bind1st(handler, this));
}

inline AsyncWatcher::AsyncWatcher(Reactor & r, Handler h) :
		Base(r, std::bind1st(h, this))
{
}

inline AsyncWatcher::AsyncWatcher(Reactor & r) :
		Base(r)
{
}

inline AsyncWatcher::~AsyncWatcher()
{
	this->stop();
}

inline void AsyncWatcher::start()
{
	ev_async_start(loop(), this);
}

inline void AsyncWatcher::stop()
{
	ev_async_stop(loop(), this);
}

inline void AsyncWatcher::set(Handler handler)
{
	Base::set(std::bind1st(handler, this));
}

inline void AsyncWatcher::send()
{
	ev_async_send(loop(), this);
}

inline bool AsyncWatcher::pending()
{
	return ev_async_pending (this);
}

inline ev_tstamp to_tstamp(struct timeval const& tv)
{
	return static_cast<double>(tv.tv_sec)
			+ static_cast<double>(tv.tv_usec) / 1000000;
}

inline struct timeval from_tstamp(ev_tstamp ts)
{
	struct timeval tv;
	tv.tv_sec = static_cast<time_t>(ts);
	tv.tv_usec = static_cast<useconds_t>((ts - tv.tv_sec) * 1000000);return
tv	;
}

inline ssize_t WriteN(int fd, void const* buf, size_t len)
{
	uint8_t const* begin = static_cast<uint8_t const*>(buf);
	size_t left = len;
	while (left > 0)
	{
		ssize_t written = ::write(fd, begin, left);
		switch (written)
		{
		case 0:
			errno = EIO;
			return -1;
		case -1:
			switch (errno)
			{
			case EINTR:
				written = 0;
				break;
			case EAGAIN:
				return len - left;
			default:
				return -1;
			}
			break;
		default:
			left -= written;
			begin += written;
			break;
		}
	}
	return len - left;
}

} // namespace net
} // namespace ads

#endif /* REACTOR_H_ */
