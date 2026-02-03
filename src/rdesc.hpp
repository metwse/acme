/**
 * @file rdesc.hpp
 * @brief RAII wrapper for rdesc C library.
 */

#pragma once
#ifndef RDESC_HPP
#define RDESC_HPP


#include "grammar.hpp"

#include <rdesc/cfg.h>
#include <rdesc/rdesc.h>

#include <memory>
#include <cstddef>


class Rdesc;

class Cfg : public std::enable_shared_from_this<Cfg> {
private:
    struct Token { explicit Token() = default; };

public:
    Cfg(Token, size_t nt_count, size_t nt_v_count, size_t nt_len,
        const rdesc_cfg_symbol *rules)
        { rdesc_cfg_init(&cfg, nt_count, nt_v_count, nt_len, rules); }

    /** SAFETY: cannot copy `struct rdesc_cfg` safely */
    Cfg(const Cfg &) = delete;

    ~Cfg()
        { rdesc_cfg_destroy(&cfg); }

    template<typename... Args>
    static auto create(Args... args)
        { return std::make_shared<Cfg>(Token {}, args...); }

    auto operator*()
        { return &cfg; }

    Rdesc new_parser();

private:
    struct rdesc_cfg cfg;
};


class Rdesc {
public:
    Rdesc(std::shared_ptr<Cfg> cfg_)
        : cfg { cfg_ }
        { rdesc_init(&p, **cfg_); }

    /** SAFETY: cannot copy `struct rdesc` safely */
    Rdesc(const Rdesc &) = delete;

    /** SAFETY: move constructor invalidates `struct rdesc` */
    Rdesc(Rdesc &&other) {
        p = other.p;
        cfg = other.cfg;
        other.destroyed = true;
    };

    ~Rdesc() {
        if (!destroyed) {
            rdesc_reset(&p, tk_destroyer);
            rdesc_destroy(&p);
        }
    }

    void start(int start_symbol)
        { rdesc_start(&p, start_symbol); }

    template<typename NodeDestroyer>
    void reset(NodeDestroyer destroyer)
        { rdesc_reset(&p, destroyer); }

    auto pump(struct rdesc_node **out, struct rdesc_cfg_token *incoming_tk)
        { return rdesc_pump(&p, out, incoming_tk); }

private:
    struct rdesc p;

    bool destroyed = false;

    std::shared_ptr<Cfg> cfg  /**< shared_ptr to Cfg class in just for preventing
                                   deletion of underlying struct rdesc_cfg. */;
};


inline Rdesc Cfg::new_parser()
    { return Rdesc(shared_from_this()); }


#endif
