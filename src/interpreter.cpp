#include "core.hpp"
#include "grammar.hpp"

#include <rdesc/cfg.h>
#include <rdesc/rdesc.h>


bool Interpreter::pump(struct rdesc_cfg_token tk) {
    struct rdesc_node *out = NULL;

    enum rdesc_result res = rdesc.pump(&out, &tk);

    switch (res) {
    case RDESC_CONTINUE:
        return true;
    case RDESC_NOMATCH:
        rdesc.reset(tk_destroyer);
        return false;
    default:
        break;
    }

    rdesc_node_destroy(out, tk_destroyer);
    return true;
}
