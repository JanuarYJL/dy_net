#ifndef DY_NET_BUFFER_H
#define DY_NET_BUFFER_H

#include <string.h>

#include <cstddef>
#include <algorithm>
#include <functional>

namespace dy
{
namespace net
{
class buffer
{
public:
    using data_type = std::string;
    using size_type = std::size_t;

    enum constant: size_type
    {
        default_size = 32 * 1024,
        per_alloc_size = 32 * 1024,
    };
    
    buffer()
    {
        data_.reserve(default_size);
        size_ = 0;
        curr_ = 0;
    }

    buffer(const char* data, const size_type& length)
    {
        data_.reserve(default_size > length ? default_size : length);
        memcpy(const_cast<char*>(data_.data()), data, length);
        size_ = length;
        curr_ = 0;
    }

    virtual ~buffer()
    {

    }

    const char* data() const
    {
        return data_.data() + curr_;
    }

    const size_type& size() const
    {
        return size_;
    }

    void set_size(const size_type& size)
    {
        size_ = size;
    }

    void pop_cache(const size_type& size)
    {
        curr_ += size;
    }

    char* writable_buff()
    {
        if (data_.capacity() <= curr_ + size_)
        {
            if (curr_ >= per_alloc_size)
            {
                move2head(curr_);
            }
            else
            {
                expand();
            }
        }
        return const_cast<char*>(data_.data() + curr_ + size_);
    }

    size_type writable_size()
    {
        return data_.capacity() - curr_ - size_;
    }

    void move2head(const size_type& off_pos)
    {
        if (off_pos == 0)
        {
            return;
        }
        if (off_pos >= size_)
        {
            size_ = 0;
            curr_ = 0;
        }
        else // there is no off_pos == 0
        {
            size_ = size_ - off_pos;
            curr_ = 0;
            memmove(const_cast<char*>(data_.data()), data() + off_pos, size_);
        }
    }

protected:
    void expand()
    {
        data_.reserve(data_.capacity() + per_alloc_size);
    }
    void shrink()
    {
        move2head(curr_);
        data_.reserve(std::max<size_type>(default_size, (size_ + per_alloc_size - 1) / per_alloc_size * per_alloc_size));
    }

protected:
    data_type data_;
    size_type curr_;  // 数据位置
    size_type size_;  // 数据长度
};


} // namespace net
} // namespace dy

#endif