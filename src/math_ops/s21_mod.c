#include "../s21_decimal.h"

void handle_exponent_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result, int *code);

int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    memset(result, 0, sizeof(*result));
    if (eq_zero(value_2))
        return S21_NAN;

    int code = ARITHMETIC_OK;

    int s1 = get_sign(value_1);

    s21_decimal copy = value_1;
    set_sign_pos(&copy);

    if (s21_is_greater(value_2, copy)) {
        set_sign_pos(&value_1);
        *result = value_1;
    } else {
        set_sign_pos(&value_1);
        set_sign_pos(&value_2);
        handle_exponent_mod(value_1, value_2, result, &code);
    }

    if (s1)
        set_sign_neg(result);

    return code;
}

void handle_exponent_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result, int *code) {
    int exp_v1 = get_exponent(value_1);
    int exp_v2 = get_exponent(value_2);

    set_exponent(&value_1, 0);
    set_exponent(&value_2, 0);

    int res_exp = exp_v1 - exp_v2;

    s21_normalize_decimal_pair(&value_1, &value_2, code);

    // Edge case. mod by 1
    if (s21_is_equal(value_2, get_power_of_ten(0)) || s21_is_equal(value_1, value_2)) {
        const s21_decimal zero = {0};
        *result = zero;
        *code = ARITHMETIC_OK;
    } else {
        // We must set 1 bit in result, because we will move this bit in mod to
        // the left
        memset(result, 0, sizeof(s21_decimal));
        result->bits[0] = 1;

        *result = s21_integer_mod(value_1, value_2);

        while (res_exp < 0) {
            *result = binary_multiplication(*result, get_power_of_ten(1), code);
            res_exp++;
        }

        set_exponent(result, res_exp);
    }
}

s21_decimal s21_integer_mod(s21_decimal dividend, s21_decimal divisor) {
    s21_decimal original_divisor = divisor;
    s21_decimal res = {0};
    s21_decimal modified_dividend = {0};
    s21_decimal zero = {0};

    if (s21_is_equal(dividend, divisor)) {
        return zero;
    } else if (s21_is_less(dividend, divisor)) {
        return dividend;
    }

    // Our goal is to align divisor & dividend, so we are shifting divisor to
    // the left

    /**
     * dividend: 0101010101010101
     * divisor: 0000000000000101 <---
     *
     * As dividend result we will get something like this:
     *
     * dividend: 0101010101010101
     * divisor: 1010000000000000 <---
     *
     * We got too far by one bit. Thus, we need to shift (divisor) to the right
     * once.
     */

    while (s21_is_less_or_equal(divisor, dividend)) {
        shiftl(&divisor);
    }

    // Shifting (divisor) once to correctly align it with dividend
    if (s21_is_less(dividend, divisor)) {
        shiftr(&divisor);
    }

    /**
     *  dividend: 0101010101010101
     *  divisor: 0101000000000000
     *
     *  An actial modision is done via substraction.
     *
     * @arg (modified_dividend) stores new value of (dividend) after
     * substraction. It will later be passed to the recursive call of sivision.
     */

    s21_sub(dividend, divisor, &modified_dividend);

    /**
     * @arg (original_divisor) is nesessary to modide by non-modified version of
     * divisor, because our original divisor was modified by shifting to the
     * left.
     */

    res = s21_integer_mod(modified_dividend, original_divisor);

    return res;
}
