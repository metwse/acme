# Boolean Script
Embriyonic boolean algebra evaluation virtual machine

See [grammar.md](grammar.md) for language definition.

```
expr half_adder(a, b) -> (sum, carry) {
    sum = (a + b) * (a * b)'
    carry = a * b
}
```

## `contribute -Wai-slop`
<img width="96" height="96" alt="no-ai-slop" align="right" src="https://github.com/user-attachments/assets/bca16d5a-a6fe-4cbf-b41f-1176e000cff2" />

Contributions are welcome! Please check our
[Code of Conduct](http://github.com/metwse/code-of-conduct) before submitting
pull requests.


## BS Utilitiy
Miscellaneous tools for interacting for Boolean script are provided in
[utils](utils/README.md).


## Future Plans
HDL-like features:
```
expr xor(a, b) -> (y) {
    y = (a + b) * (a * b)'
}

expr full_adder(a, b, cin) -> (sum, cout) {
    ab = xor(a, b)
    sum = xor(ab, cin)
    cout = (a * b) + (cin * (a + b))
}

sync {
    vec<4> cnt = [0, 0, 0, 0]

    @clk {
        cnt0, c0 = full_adder(cnt[0], 1, 0)
        cnt1, c1 = full_adder(cnt[1], 0, c0)
        cnt2, c2 = full_adder(cnt[2], 0, c1)
        cnt3, c3 = full_adder(cnt[3], 0, c2)

        cnt = [cnt0, cnt1, cnt2, cnt3]
    }
}
```
...electronics simulation using Boolean script
