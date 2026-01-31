#include "grammar.hpp"
#include "rdesc.hpp"

#include <memory>

#include <rdesc/cfg.h>


std::shared_ptr<Cfg> load_grammar() {
    return Cfg::create(
        NT_COUNT, NT_VARIANT_COUNT, NT_BODY_LENGTH,
        (const rdesc_cfg_symbol *)(grammar)
    );
}
