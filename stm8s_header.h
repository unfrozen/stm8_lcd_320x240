/*
 * Choose stm8s_103 or stm8s_105 header,
 * depending on compile directive.
 */

#ifdef STM8103
#include "stm8_103.h"
#include "stm8_103.inc"
#endif

#ifdef STM8105
#include "stm8_105.h"
#include "stm8_105.inc"
#endif

#include "vectors.h"

