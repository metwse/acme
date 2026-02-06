#include <rdesc/rdesc.h>

#include <type_traits>
#include <memory>
#include <vector>

using std::is_invocable_v;
using std::vector;
using std::unique_ptr;


template<typename T>
static inline auto get_seminfo(struct rdesc_node *tk) {
    return unique_ptr<T>(static_cast<T *>(tk->tk.seminfo));
}

template<typename Fn>
static void traverse_rrr_list(struct rdesc_node *ls, Fn process) {
    while (true) {
        auto next = ls->nt.children[1];

        auto *entry = ls->nt.children[0];
        auto *delim = next->nt.child_count ? next->nt.children[0] : nullptr;

        if constexpr (is_invocable_v<Fn, decltype(entry), decltype(delim)>)
            process(entry, delim);
        else
            process(entry);

        if (next->nt.child_count == 0)
            break;
        ls = next->nt.children[1];
    }
}

template<typename T>
static auto get_rrr_seminfo(struct rdesc_node *ls) {
    vector<unique_ptr<T>> res;

    traverse_rrr_list(ls, [&](auto *entry) {
        res.push_back(get_seminfo<T>(entry));
    });

    return res;
}
