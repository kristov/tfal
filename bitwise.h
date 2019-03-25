#ifndef BITWISE_H
#define BITWISE_H

#define BIT_TEST(a, f)   ((a >> f) & 1)
#define BIT_SET(a, f)    (a |= (1 << f))
#define BIT_UNSET(a, f)  (a &= ~(1 << f))

#endif
