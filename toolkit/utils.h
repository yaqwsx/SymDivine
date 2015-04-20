#pragma once

#include <functional>
#include <string>
#include <fstream>
#include <map>
#include <memory>
#include <chrono>
#include <vector>
#include <llvmsym/formula/rpn.h>

/**
 * Inserts newline char every n characters
 */
void break_string(size_t n, std::string& s);

/**
 * Provides debug output stream
 */
class DebugInterface {
public:
    static void add_stream(const std::string& name, std::unique_ptr<std::ostream>& s) {
        streams[name] = std::move(s);
    }

    static void add_file(const std::string& name, const std::string& file) {
        streams[name] = std::unique_ptr<std::ostream>(new std::ofstream(file));
    }

    static std::ostream& get_stream(const std::string& name) {
        auto res = streams.find(name);
        if (res == streams.end())
            throw std::runtime_error("DebugInterface: No such stream");
        return *(res->second);
    }

private:
    static std::map<std::string, std::unique_ptr<std::ostream>> streams;
};

/**
 * Provides interface for time measuremnts
 */
class StopWatch {
public:
    void start() { start_time = std::chrono::high_resolution_clock::now(); }
    void stop() { end_time = std::chrono::high_resolution_clock::now(); }
    size_t getUs() {
        return std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    }
    size_t getMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    }
    size_t getS() {
        return std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    }
private:
    typedef typename std::chrono::high_resolution_clock::time_point time_point;
    time_point start_time;
    time_point end_time;
};

// adapted from boost::hash_combine
std::size_t hash_comb(std::size_t h, std::size_t v);

namespace std {

    template <class A, class B>
    struct hash <pair<A, B>> {
        size_t operator() (const pair<A, B>& p) const {
            return hash_comb(hash<A>()(p.first), hash<B>()(p.second));
        }
    };

    template <class T>
    struct hash<vector<T>> {
        size_t operator() (const vector<T>& v) const {
            size_t h = 0;
            for (const T& t : v)
                h = hash_comb(h, hash<T>()(t));
            return h;
        }
    };
}

namespace std {
    template <>
    struct hash<llvm_sym::Formula::Ident> {
        size_t operator() (const llvm_sym::Formula::Ident& i) const {
            size_t h = 0;
            h = hash_comb(h, hash<short unsigned>()(i.seg));
            h = hash_comb(h, hash<short unsigned>()(i.off));
            h = hash_comb(h, hash<short unsigned>()(i.gen));
            h = hash_comb(h, hash<unsigned char>()(i.bw));
            return h;
        }
    };

    template <>
    struct hash<llvm_sym::Formula::Item> {
        size_t operator() (const llvm_sym::Formula::Item& i) const {
            switch (i.kind) {
            case llvm_sym::Formula::Item::Kind::Op:
                return hash<std::underlying_type<llvm_sym::Formula::Item::Operator>::type>()(i.op);
            case llvm_sym::Formula::Item::Kind::BoolVal:
            case llvm_sym::Formula::Item::Kind::Constant:
                return hash<int>()(i.value);
            case llvm_sym::Formula::Item::Kind::Identifier:
                return hash<llvm_sym::Formula::Ident>()(i.id);
            default:
                assert(false);
            }
        };
    };

    template <>
    struct hash<llvm_sym::Formula> {
        size_t operator() (const llvm_sym::Formula& f) const {
            return hash<vector<llvm_sym::Formula::Item>>()(f._rpn);
        }
    };

    template<>
    struct equal_to<llvm_sym::Formula> {
        bool operator()(const llvm_sym::Formula& a, const llvm_sym::Formula& b) const {
            return a._rpn == b._rpn;
        }
    };

    template<>
    struct equal_to<pair<llvm_sym::Formula, llvm_sym::Formula>> {
        bool operator()(const pair<llvm_sym::Formula, llvm_sym::Formula>& a,
            const pair<llvm_sym::Formula, llvm_sym::Formula>& b) const
        {
            return a.first._rpn == b.first._rpn
                && a.second._rpn == b.second._rpn;
        }
    };
}