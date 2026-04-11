/* calc.x */
struct numbers {
    int a;
    int b;
};

program CALC_PROG {
    version CALC_VERS {
        int ADD(numbers) = 1;
    } = 1;
} = 0x23456789;