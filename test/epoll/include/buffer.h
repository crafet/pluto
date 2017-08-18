

// for uint8_t
#include <stdint.h>

#include <vector>
#include <algorithm>

// for assert
#include <assert.h>

// for memcpy
#include <string.h>
#include <stdio.h>

typedef uint8_t byte;

using namespace std;
namespace pluto {

// buffer class
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
        // adjust the reader_index
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
		// left space is enough
		if (writable() >= more)
			return;

        // all buffer is less than more
        // have to resize the buffer
		if (writable() + prependable() < more) {
			data_.resize(writer_index_ + more);
            printf("resize more: %d\n", more);

		} else {
            // right now total buffer is enough ,but left space is not enough
            // then adjust the reader_index and writer_index to satisfy more
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

}
