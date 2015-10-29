#include <cstring>
#include <cstdio>

//A simple class of ring buffer for raw binary data.
class RingBuffer
{
public:
	RingBuffer(size_t cap) :
		write_(0), 
		read_(0),
		size_(0),
		capacity_(cap)
	{
		data_ = new char [capacity_];
		memset(data_, 0, capacity_);
	}

	~RingBuffer()
	{
		delete [] data_;
	}

	// how many bytes are left in buffer
	size_t size() const
	{
		return size_;
	}
	
	// max size of the buffer
	size_t capacity() const
	{
		return capacity_;
	}

	// how many bytes can be written
	size_t left() const
	{
		return capacity_ - size_;
	}

	// write len bytes to buffer, return how many bytes have been written actually.
	size_t write(const char* src, size_t len)
	{
		if (len == 0)
			return 0;

		len = std::min(len, left());
		size_t tail_left = capacity_ - write_;
		if (tail_left < len)
		{
			write_impl(src, tail_left);
			write_impl(src + tail_left, len - tail_left);
		}
		else
		{
			write_impl(src, len);
		}
		return len;
	}

	// read len bytes from buffer, return how many bytes have been read actually.
	size_t read(char* dst, size_t len)
	{
		if (len == 0)
			return 0;

		len = std::min(len, size());
		size_t tail_left = capacity_ - read_;

		if (len <= tail_left)
		{
			read_impl(dst, len);
		}
		else
		{
			read_impl(dst, tail_left);
			read_impl(dst + tail_left, len - tail_left);
		}
		return len;
	}
private:
	void write_impl(const char* src, size_t len)
	{
		memcpy(data_ + write_, src, len);
		size_ += len;
		write_ = (write_ + len) % capacity_;
	}
	void read_impl(char* dst, size_t len)
	{
		memcpy(dst, data_ + read_, len);
		size_ -= len;
		read_ = (read_ + len) % capacity_; 
	}

	size_t write_;
	size_t read_;
	size_t size_;
	size_t capacity_;

	char* data_;
};
