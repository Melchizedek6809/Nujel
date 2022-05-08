#ifndef NUJEL_API
#define NUJEL_API
/* The API is still very unstable, still this is the
 * header that should be included if you want to link
 * Nujel into your program.
 */

#include "nujel.h"

#include "allocation/allocator.h"
#include "allocation/garbage-collection.h"
#include "allocation/roots.h"
#include "allocation/symbol.h"

#include "collection/list.h"
#include "collection/string.h"
#include "collection/tree.h"

#include "misc/pf.h"
#include "operation.h"

#include "reader.h"

#include "display.h"
#include "exception.h"
#include "type-system.h"

#include "type/closure.h"
#include "type/symbol.h"
#include "type/val.h"

#endif
