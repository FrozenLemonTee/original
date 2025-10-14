#ifndef FILTERSTREAM_H
#define FILTERSTREAM_H

#include "filter.h"
#include "chain.h"
#include "refCntPtr.h"


/**
* @file filterStream.h
* @brief Composite filter operations with logical chaining
* @details Implements a stream-like structure for combining multiple filters through logical operators.
* Supports AND/OR/NOT operations and explicit grouping via group() function.
* Uses postfix notation for internal evaluation and avoids direct parenthesis usage.
*/

namespace original{
    /**
    * @class filterStream
    * @tparam TYPE Element type for filtering
    * @brief Composite filter builder with logical operator chaining
    * @details Enables construction of complex filter conditions through operator overloading.
    * Maintains internal operator precedence using postfix conversion. Use group() for explicit
    * precedence control instead of parentheses.
    */
    template<typename TYPE>
    class filterStream
    {
    public:
        /// @internal Operator types for postfix conversion
        enum class opts{AND = 1, OR = 0, NOT = 2, LEFT_BRACKET = 3, RIGHT_BRACKET = 4};
    private:
        ///< Null filter instance used as operator placeholder in stream processing
        static const strongPtr<filter<TYPE>> nullFilter;

        mutable chain<strongPtr<filter<TYPE>>> stream{}; ///< Filter operand chain
        mutable chain<opts> ops{}; ///< Operator sequence storage
        mutable bool flag; ///< Postfix conversion status flag

    protected:
        /**
        * @brief Protected constructor for stream initialization
        * @note Used internally and by friend functions
        */
        explicit filterStream();

        /**
        * @brief Add bracket operators for grouping
        * @details Wraps current stream in logical parentheses
        */
        void addBrackets();

        /**
        * @brief Add AND operator to the stream
        * @details Inserts logical AND between subsequent filters
        */
        void addAndOpt();

        /**
        * @brief Add OR operator to the stream
        * @details Inserts logical OR between subsequent filters
        */
        void addOrOpt();

        /**
        * @brief Add NOT operator to the stream
        * @details Negates subsequent filter condition
        */
        void addNotOpt();

        /**
        * @brief Append filter to the stream
        * @param f Filter to add to the end of stream
        */
        void pushEnd(const filter<TYPE>& f);

        /**
        * @brief Merge another filterStream into this one
        * @param fs Source filterStream to merge
        */
        void pushAll(const filterStream& fs);

        /**
        * @brief Convert infix notation to postfix
        * @details Internal implementation of Shunting Yard algorithm
        */
        void toPostFix() const;

    public:
        ~filterStream() = default;

        /**
        * @brief AND operator with single filter
        * @param f Filter to AND with current stream
        * @return Reference to modified filterStream
        */
        filterStream& operator&&(const filter<TYPE>& f);

        /**
        * @brief AND operator with another filterStream
        * @param fs filterStream to AND with current stream
        * @return Reference to modified filterStream
        */
        filterStream& operator&&(const filterStream& fs);

        /**
        * @brief OR operator with single filter
        * @param f Filter to OR with current stream
        * @return Reference to modified filterStream
        */
        filterStream& operator||(const filter<TYPE>& f);

        /**
        * @brief OR operator with another filterStream
        * @param fs filterStream to OR with current stream
        * @return Reference to modified filterStream
        */
        filterStream& operator||(const filterStream& fs);

        /**
        * @brief Logical NOT operator
        * @return Reference to negated filterStream
        * @note Automatically adds grouping parentheses
        */
        filterStream& operator!();

        /**
        * @brief Filter evaluation operator
        * @param t Element to test against filter conditions
        * @return bool True if element passes filter chain
        */
        bool operator()(const TYPE& t) const;

        // Friend operator implementations
        template<typename T>
        friend filterStream<T> operator&&(const filter<T>& f1, const filter<T>& f2);
        template<typename T>
        friend filterStream<T> operator&&(const filter<T>& f, const filterStream<T>& ofs);
        template<typename T>
        friend filterStream<T> operator&&(const filterStream<T>& ofs, const filter<T>& f);
        template<typename T>
        friend filterStream<T> operator||(const filter<T>& f1, const filter<T>& f2);
        template<typename T>
        friend filterStream<T> operator||(const filter<T>& f, const filterStream<T>& ofs);
        template<typename T>
        friend filterStream<T> operator||(const filterStream<T>& ofs, const filter<T>& f);
        template<typename T>
        friend filterStream<T> operator!(const filter<T>& f);
        template<typename T>
        friend filterStream<T> operator!(const filterStream<T>& ofs);
        template<typename T>
        friend filterStream<T> group(const filterStream<T>& ofs);
        template<typename T>
        friend filterStream<T> group(const filter<T>& f);
    };

    template <typename TYPE>
    const strongPtr<filter<TYPE>> filterStream<TYPE>::nullFilter{};

    /**
    * @brief Create AND filterStream from two filters
    * @tparam T Element type
    * @param f1 First filter operand
    * @param f2 Second filter operand
    * @return New filterStream with AND operation
    */
    template<typename T>
    filterStream<T> operator&&(const filter<T>& f1, const filter<T>& f2);

    /**
    * @brief AND operator between filter and filterStream
    * @tparam T Element type
    * @param f Filter operand
    * @param ofs filterStream operand
    * @return New combined filterStream
    */
    template<typename T>
    filterStream<T> operator&&(const filter<T>& f, const filterStream<T>& ofs);

    /**
    * @brief AND operator between filterStream and filter
    * @tparam T Element type
    * @param ofs filterStream operand
    * @param f Filter operand
    * @return New combined filterStream
    */
    template<typename T>
    filterStream<T> operator&&(const filterStream<T>& ofs, const filter<T>& f);

    /**
    * @brief Create OR filterStream from two filters
    * @tparam T Element type
    * @param f1 First filter operand
    * @param f2 Second filter operand
    * @return New filterStream with OR operation
    */
    template<typename T>
    filterStream<T> operator||(const filter<T>& f1, const filter<T>& f2);

    /**
    * @brief OR operator between filter and filterStream
    * @tparam T Element type
    * @param f Filter operand
    * @param ofs filterStream operand
    * @return New combined filterStream
    */
    template<typename T>
    filterStream<T> operator||(const filter<T>& f, const filterStream<T>& ofs);

    /**
    * @brief OR operator between filterStream and filter
    * @tparam T Element type
    * @param ofs filterStream operand
    * @param f Filter operand
    * @return New combined filterStream
    */
    template<typename T>
    filterStream<T> operator||(const filterStream<T>& ofs, const filter<T>& f);

    /**
    * @brief Create negated filterStream from filter
    * @tparam T Element type
    * @param f Filter to negate
    * @return New filterStream with NOT operation
    * @note Automatically adds grouping parentheses
    */
    template<typename T>
    filterStream<T> operator!(const filter<T>& f);

    /**
    * @brief Create negated filterStream from existing stream
    * @tparam T Element type
    * @param ofs filterStream to negate
    * @return New negated filterStream
    * @note Automatically adds grouping parentheses
    */
    template<typename T>
    filterStream<T> operator!(const filterStream<T>& ofs);

    /**
    * @brief Create grouped filterStream from existing stream
    * @tparam T Element type
    * @param ofs filterStream to group
    * @return New grouped filterStream
    * @details Use instead of parentheses for explicit precedence control
    */
    template<typename T>
    filterStream<T> group(const filterStream<T>& ofs);

    /**
    * @brief Create grouped filterStream from single filter
    * @tparam T Element type
    * @param f Filter to group
    * @return New grouped filterStream
    * @details Enables future operator precedence modifications
    */
    template<typename T>
    filterStream<T> group(const filter<T>& f);
} // namespace original

    template <typename TYPE>
    original::filterStream<TYPE>::filterStream() : stream(), ops(), flag(false) {}

    template <typename TYPE>
    auto original::filterStream<TYPE>::addBrackets() -> void
    {
        this->stream.pushBegin(nullFilter);
        this->stream.pushEnd(nullFilter);
        this->ops.pushBegin(opts::LEFT_BRACKET);
        this->ops.pushEnd(opts::RIGHT_BRACKET);
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::addAndOpt() -> void
    {
        this->stream.pushEnd(nullFilter);
        this->ops.pushEnd(opts::AND);
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::addOrOpt() -> void
    {
        this->stream.pushEnd(nullFilter);
        this->ops.pushEnd(opts::OR);
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::addNotOpt() -> void
    {
        this->stream.pushBegin(nullFilter);
        this->ops.pushBegin(opts::NOT);
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::pushEnd(const filter<TYPE>& f) -> void
    {
        this->stream.pushEnd(strongPtr<filter<TYPE>>(f.clone()));
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::pushAll(const filterStream& fs) -> void
    {
        for (auto& filter: fs.stream)
        {
            this->stream.pushEnd(filter);
        }
        for (auto& op: fs.ops)
        {
            this->ops.pushEnd(op);
        }
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::toPostFix() const -> void{
        this->flag = true;

        chain<strongPtr<filter<TYPE>>> stream_post;
        chain<opts> ops_post;
        chain<opts> ops_tmp;

        auto it_stream = this->stream.begins();
        auto it_ops = this->ops.begins();

        while (it_stream->isValid()){
            if (it_stream->get() != nullFilter){
                stream_post.pushEnd(it_stream->get());
            } else if (it_ops->isValid()){
                switch (it_ops->get()) {
                    case opts::LEFT_BRACKET:
                        ops_tmp.pushEnd(opts::LEFT_BRACKET);
                        break;
                    case opts::RIGHT_BRACKET:
                        while (!ops_tmp.empty() && ops_tmp[-1] != opts::LEFT_BRACKET){
                            stream_post.pushEnd(nullFilter);
                            ops_post.pushEnd(ops_tmp.popEnd());
                        }
                        ops_tmp.popEnd();
                        break;
                    case opts::NOT:
                        ops_tmp.pushEnd(opts::NOT);
                        break;
                    default:
                        while (!ops_tmp.empty()
                               && ops_tmp[-1] >= it_ops->get()
                               && ops_tmp[-1] != opts::LEFT_BRACKET){
                            stream_post.pushEnd(nullFilter);
                            ops_post.pushEnd(ops_tmp.popEnd());
                        }
                        ops_tmp.pushEnd(it_ops->get());
                        break;
                }
                it_ops->next();
            }
            it_stream->next();
        }

        while (!ops_tmp.empty()){
            stream_post.pushEnd(nullFilter);
            ops_post.pushEnd(ops_tmp.popEnd());
        }
        this->stream = stream_post;
        this->ops = ops_post;
        delete it_ops;
        delete it_stream;
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::operator&&(const filter<TYPE>& f) -> filterStream&
    {
        this->addAndOpt();
        this->pushEnd(f);
        return *this;
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::operator&&(const filterStream& fs) -> filterStream&
    {
        this->addAndOpt();
        this->pushAll(fs);
        return *this;
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::operator||(const filter<TYPE>& f) -> filterStream&
    {
        this->addOrOpt();
        this->pushEnd(f);
        return *this;
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::operator||(const filterStream& fs) -> filterStream&
    {
        this->addOrOpt();
        this->pushAll(fs);
        return *this;
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::operator!() -> filterStream&
    {
        this->addBrackets();
        this->addNotOpt();
        return *this;
    }

    template <typename TYPE>
    auto original::operator&&(const filter<TYPE> &f1, const filter<TYPE> &f2) -> filterStream<TYPE> {
        filterStream<TYPE> fs;
        fs.pushEnd(f1);
        fs.addAndOpt();
        fs.pushEnd(f2);
        return fs;
    }

    template <typename TYPE>
    auto original::operator&&(const filter<TYPE> &f, const filterStream<TYPE> &ofs) -> filterStream<TYPE> {
        filterStream<TYPE> fs;
        fs.pushEnd(f);
        fs.addAndOpt();
        fs.pushAll(ofs);
        return fs;
    }

    template<typename TYPE>
    auto original::operator&&(const filterStream<TYPE> &ofs, const filter<TYPE> &f) -> filterStream<TYPE> {
        filterStream<TYPE> fs(ofs);
        return fs && f;
    }

    template <typename TYPE>
    auto original::operator||(const filter<TYPE> &f1, const filter<TYPE> &f2) -> filterStream<TYPE> {
        filterStream<TYPE> fs;
        fs.pushEnd(f1);
        fs.addOrOpt();
        fs.pushEnd(f2);
        return fs;
    }

    template <typename TYPE>
    auto original::operator||(const filter<TYPE> &f, const filterStream<TYPE> &ofs) -> filterStream<TYPE> {
        filterStream<TYPE> fs;
        fs.pushEnd(f);
        fs.addOrOpt();
        fs.pushAll(ofs);
        return fs;
    }

    template<typename TYPE>
    auto original::operator||(const filterStream<TYPE> &ofs, const filter<TYPE> &f) -> filterStream<TYPE> {
        filterStream<TYPE> fs(ofs);
        return fs || f;
    }

    template <typename TYPE>
    auto original::operator!(const filter<TYPE> &f) -> filterStream<TYPE> {
        filterStream<TYPE> fs;
        fs.pushEnd(f);
        fs.addNotOpt();
        return fs;
    }

    template<typename TYPE>
    auto original::operator!(const filterStream<TYPE> &ofs) -> filterStream<TYPE> {
        filterStream<TYPE> fs(ofs);
        return !fs;
    }

    template<typename TYPE>
    auto original::group(const filterStream<TYPE> &ofs) -> filterStream<TYPE> {
        filterStream<TYPE> fs;
        fs.pushAll(ofs);
        fs.addBrackets();
        return fs;
    }

    template<typename TYPE>
    auto original::group(const filter<TYPE> &f) -> filterStream<TYPE> {
        filterStream<TYPE> fs;
        fs.pushEnd(f);
        return fs;
    }

    template <typename TYPE>
    auto original::filterStream<TYPE>::operator()(const TYPE &t) const -> bool {
        if (!this->flag) this->toPostFix();

        chain<bool> value_stack;
        auto it_stream = this->stream.begins();
        auto it_ops = this->ops.begins();

        while (it_stream->isValid()){
            if (it_stream->get() != nullFilter){
                value_stack.pushEnd(it_stream->get()->operator()(t));
            } else if (it_ops->isValid()){
                switch (it_ops->get()) {
                    case opts::NOT:
                        value_stack[-1] = !value_stack[-1];
                        break;
                    default:
                        const bool right = value_stack.popEnd();
                        const bool left = value_stack.popEnd();
                        value_stack.pushEnd(it_ops->get() == opts::AND ?
                                                left && right : left || right);
                        break;
                }
                it_ops->next();
            }
            it_stream->next();
        }
        delete it_ops;
        delete it_stream;
        return value_stack[-1];
    }

#endif //FILTERSTREAM_H
