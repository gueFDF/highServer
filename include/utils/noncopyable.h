#ifndef NONCOPYABLE
#define NONCOPYABLE
namespace tinyrpc {

class noncopyable {
public:
    // 禁用默认复制和默认拷贝
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
} // namespace tinyrpc

#endif /* NONCOPYABLE */