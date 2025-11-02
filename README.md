# Boolean Script
Embriyonic boolean algebra evaluation virtual machine

```
expr half_adder(a, b) -> (sum, carry) {
    sum = (a + b) * (a * b)'
    carry = a * b
}
```

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
    vector[4] reg = [0, 0, 0, 0]

    @clk {
        reg0, c0 = full_adder(reg[0], 1, 0)
        reg1, c1 = full_adder(reg[1], 0, c0)
        reg2, c2 = full_adder(reg[2], 0, c1)
        reg3, c3 = full_adder(reg[3], 0, c2)

        reg = [reg0, reg1, reg2, reg3]
    }
}
```
...electronics simulation using Boolean script
