// Dictionary selection; generated tables are kept in separate files.
#pragma once

#ifdef AUTOCORRECT_FULL_DICTIONARY
#include "autocorrect_data_full.h"
#elif defined(QWERTY_ENABLE)
// QWERTY's extra layer/code needs a smaller dictionary on rev1.
#include "autocorrect_data_qwerty.h"
#else
#include "autocorrect_data_reduced.h"
#endif
