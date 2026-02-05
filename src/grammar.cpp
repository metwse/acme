#include "../include/grammar.hpp"
#include "../include/lex.hpp"
#include "../include/rdesc.hpp"

#include <rdesc/cfg.h>

#include <memory>

using std::weak_ptr, std::shared_ptr;


shared_ptr<Cfg> global_cfg() {
    static weak_ptr<Cfg> global_cfg;

    if (global_cfg.expired()) {
        shared_ptr<Cfg> cfg = Cfg::create(
            NT_COUNT, NT_VARIANT_COUNT, NT_BODY_LENGTH,
            (const rdesc_cfg_symbol *)(grammar)
        );

        global_cfg = cfg;

        return shared_ptr(global_cfg);
    } else {
        return shared_ptr(global_cfg);
    }
}

void tk_printer(const struct rdesc_cfg_token *tk, FILE *out) {
    if (tk->id == TK_IDENT) {
        auto seminfo = reinterpret_cast<IdentInfo *>(tk->seminfo);

        fprintf(out, "{{ident|%zu}}", seminfo->id);
    } else if (tk->id == TK_NUM) {
        auto seminfo = reinterpret_cast<NumInfo *>(tk->seminfo);

        fprintf(out, "{{num|base: %d, %s}}",
                seminfo->base, seminfo->num.c_str());
    } else {
        /* ignoring seminfo of table_value */
        fprintf(out, "%s", tk_names_escaped[tk->id]);
    }
}

void tk_destroyer(struct rdesc_cfg_token *tk) {
    auto seminfo = reinterpret_cast<SemInfo *>(tk->seminfo);

    delete seminfo;
}

