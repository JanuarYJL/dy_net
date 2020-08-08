#ifndef DY_NET_MACRO_H
#define DY_NET_MACRO_H

// DISABLE_INIT_COPY_AND_ASSIGN def
#ifndef DISABLE_INIT_COPY_ASSIGN
#define DISABLE_INIT_COPY_ASSIGN(T) \
    T() = delete;                    \
    T(const T &) = delete;           \
    T &operator=(const T &) = delete;
#endif

// DISABLE_COPY_ASSIGN def
#ifndef DISABLE_COPY_ASSIGN
#define DISABLE_COPY_ASSIGN(T) \
    T(const T &) = delete;      \
    T &operator=(const T &) = delete;
#endif

#endif DY_NET_MACRO_H