#pragma once
/* The API is still very unstable, still this is the
 * header that should be included if you want to link
 * Nujel into your program.
 */

#include "nujel.h"

#include "allocation/array.h"
#include "allocation/closure.h"
#include "allocation/garbage-collection.h"
#include "allocation/roots.h"
#include "allocation/string.h"
#include "allocation/val.h"

#include "collection/list.h"
#include "collection/string.h"
#include "collection/tree.h"

#include "operation/string.h"
#include "operation/special.h"

#include "s-expression/reader.h"
#include "s-expression/writer.h"

#include "display.h"
#include "exception.h"
#include "type-system.h"

#include "type/closure.h"
#include "type/native-function.h"
#include "type/symbol.h"
#include "type/val.h"
