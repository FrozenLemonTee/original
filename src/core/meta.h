#ifndef ORIGINAL_META_H
#define ORIGINAL_META_H

namespace original
{
    class defaultCopy
    {
    protected:
        defaultCopy() noexcept = default;
        defaultCopy(const defaultCopy&) noexcept = default;
        defaultCopy& operator=(const defaultCopy&) noexcept = default;
    };

    class defaultMove
    {
    protected:
        defaultMove() noexcept = default;
        defaultMove(defaultMove&&) noexcept = default;
        defaultMove& operator=(defaultMove&&) noexcept = default;
    };

    class noCopy
    {
    protected:
        noCopy() noexcept = default;
    public:
        noCopy(const noCopy&) noexcept = delete;
        noCopy& operator=(const noCopy&) noexcept = delete;
    };

    class noMove
    {
    protected:
        noMove() noexcept = default;
    public:
        noMove(noMove&&) noexcept = delete;
        noMove& operator=(noMove&&) noexcept = delete;
    };

    class defaultMeta : public defaultCopy, public defaultMove
    {
    protected:
        defaultMeta() noexcept = default;
    };

    class noMeta : public noCopy, public noMove
    {
    protected:
        noMeta() noexcept = default;
    };

    class moveOnlyMeta : public noCopy, public defaultMove
    {
    protected:
        moveOnlyMeta() noexcept = default;
    };

    class copyOnlyMeta : public defaultCopy, public noMove
    {
    protected:
        copyOnlyMeta() noexcept = default;
    };
}

#endif //ORIGINAL_META_H