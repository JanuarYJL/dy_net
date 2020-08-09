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
    }

    buffer(const char* data, const size_type& length)
    {
        data_.reserve(default_size > length ? default_size : length);
        memcpy(const_cast<char*>(data_.data()), data, length);
        size_ = length;
    }

    virtual ~buffer()
    {

    }

    const char* data() const
    {
        return data_.data();
    }

    const size_type& size() const
    {
        return size_;
    }

    void set_size(const size_type& size)
    {
        size_ = size;
    }

    char* writable_buff()
    {
        if (data_.capacity() <= size_)
        {
            expand();
        }
        return const_cast<char*>(data_.data() + size_);
    }

    size_type writable_size()
    {
        return data_.capacity() - size_;
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
        }
        else // there is no off_pos == 0
        {
            size_ = size_ - off_pos;
            memmove(mut_data(), data() + off_pos, size_);
        }
    }

protected:
    char* mut_data()
    {
        return const_cast<char*>(data_.data());
    }
    void expand()
    {
        data_.reserve(data_.capacity() + per_alloc_size);
    }
    void shrink()
    {
        data_.reserve(std::max<size_type>(default_size, (size_ + per_alloc_size - 1) / per_alloc_size * per_alloc_size));
    }

protected:
    data_type data_;
    size_type size_;
};


} // namespace net
} // namespace dy

#endif DY_NET_BUFFER_H