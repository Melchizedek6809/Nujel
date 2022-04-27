// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

/**
 * Author: Koh Zi Han, based on implementation by Koh Zi Chun
 * Improved by: Jakub T. Jankiewicz
 */

(function(mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("../../lib/codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror"], mod);
  else // Plain browser env
    mod(CodeMirror);
})(function(CodeMirror) {
"use strict";

CodeMirror.defineMode("nujel", function () {
    var BUILTIN = "builtin", COMMENT = "comment", STRING = "string",
        SYMBOL = "symbol", ATOM = "atom", NUMBER = "number", BRACKET = "bracket";
    var INDENT_WORD_SKIP = 2;

    function makeKeywords(str) {
        var obj = {}, words = str.split(" ");
        for (var i = 0; i < words.length; ++i) obj[words[i]] = true;
        return obj;
    }

    var keywords = makeKeywords("quote array/new if cond do - tree/new quasiquote unquote unquote-splicing bool int float vec string macro memory-info garbage-collect val->index index->val sym->index index->sym % / * + pow add sub mul div mod add/int sub/int mul/int div/int mod/int pow/int abs sqrt cbrt floor ceil round sin cos tan atan2 vec/magnitude array/ref array/length array/length! array/set! array/allocate logand logior logxor lognot ash popcount int->bytecode-op bytecode-op->int arr->bytecode-arr bytecode-arr->arr bytecode-eval resolve resolves? closure closure-parent closure-caller closure! current-closure current-lambda symbol-search symbol-count symbol-table* def set! let* macro* fn* ω* environment* list apply macro-apply eval* car cdr cons nreverse < <= == != >= > nil? keyword? read and or while try throw cat trim string/length uppercase lowercase capitalize string/cut index-of last-index-of char-at from-char-code str->sym sym->str str/write time time/milliseconds tree/ref tree/list tree/keys tree/values tree/get-list tree/size tree/has? tree/set! tree/dup tree/key* tree/value* tree/left* tree/right* symbol->keyword keyword->symbol type-of vec/x vec/y vec/z vec/dot vec/normalize vec/reflect System/OS System/Architecture lognand bit-set?! zero? not bit-clear?! array/+= array/++ array/fill! array/append array? array/dup array/reduce array/map array/filter array/equal? equal? array/push array/swap array/heapify array/make-heap array/heap-sort array/sort array/cut max min array/2d/allocate array/2d/fill! array/2d/ref array/2d/set! array/2d/print display newline avl/empty avl/empty? avl/default-cmp avl/typecheck avl/root avl/key avl/tree avl/height avl/left avl/right avl/cmp avl/min-node avl/update-left avl/update-right avl/update-key avl/update-root avl/update-height avl/rotate-right avl/rotate-left avl/balance avl/insert-rebalance avl/node-insert avl/insert avl/node-get avl/get avl/from-list list/reduce avl/remove-rebalance avl/node-remove avl/remove avl/equal-node? avl/equal? avl/reduce-node avl/reduce avl/reduce-node-bin avl/reduce-bin avl/map avl/map-to avl/to-list sum reduce join for-each count delete filter remove flatten/λ collection? append flatten ref list/ref tree/filter list/filter tree/reduce length list/length map list/map sort list/sort/merge member list/member cut list/cut except-last-pair/iter reverse except-last-pair last-pair make-list range pos? sublist neg? list-head list-tail getf cadr list/sort/bubble list/merge-sorted-lists list/split-half-rec cddr list/split-half list/sort list/equal? pair? list/take list/drop tree/zip tree/+= tree/-= tree/++ tree/-- tree/equal? tree? val->bytecode-op >> sym->bytecode-op int-fit-in-byte? $nop $ret $push/int/byte $push/int $push/lval $add/int $debug/print-stack $push/symbol $make-list $eval $apply $< $<= $== $>= $> $apply/dynamic $call $try $throw $jmp $jt $jf $dup $drop $def $set $get $fn $macro* $closure/push $closure/enter $let $closure/pop $roots/save $roots/restore $push/nil $swap assemble/build-sym-map assemble/relocate-op cadddr caddr assemble/emit-relocated-ops assemble/verbose assemble* ansi-blue println ansi-yellow ansi-green assemble asmrun bytecompile/gen-label/counter bytecompile/gen-label bytecompile/literal bytecompile/quote bytecompile/do/form bytecompile* last? bytecompile/do bytecompile/procedure bytecompile/def symbol? bytecompile/set! bytecompile/if bytecompile/while bytecompile/procedure/arg bytecompile/procedure/dynamic bytecompile/and/rec bytecompile/and bytecompile/or/rec bytecompile/or bytecompile/string bytecompile/array bytecompile/tree bytecompile/fn* bytecompile/macro* bytecompile/ω* bytecompile/let* bytecompile/try bytecompile byterun -> compile compile/environment compile/verbose compile/do/args compile* compile/do compile/def compile/set! compile/fn* caddddr compile/macro* compile/ω* compile/try compile/if compile/let* compile/map compile/while compile/macro eval-in load/forms compile/forms defmacro string? fn defn ω defobj eval eval-compile display/error read-eval-compile eval-load read-eval-load typecheck/only when-not disassemble/length bytecode/nil-catcher error bytecode-op->val bytecode-arr->val bytecode-op->sym bytecode-arr->sym bytecode-op->offset bytecode-arr->offset disassemble/op disassemble/array disassemble/bytecode-array disassemble/print string/pad-start disassemble disassemble/test ansi-red yield-queue yield yield-run timeout event-bind event-clear event-fire let/arg let/args let if-let when-let comment += cdr! identity default caar cdar cadar cdddr keyword->string string->keyword if-not when case/clauses/multiple case/clauses case gensym for for-in thread/-> thread/->> ->> returnable/λ return returnable numeric? int? float? vec? zero-neg? odd? even? not-zero? inequal? bool? object? macro? lambda? native? special-form? procedure? bytecode-array? bytecode-op? in-range? quasiquote-real describe/closure stacktrace time/seconds time/minutes time/hours profile-form profile hash/adler32 PI π ++ -- +x fib wrap-value +1 radians display/error/wrap display/error/iter describe/thing describe/string describe mem ansi-white ansi-pink ansi-reset symbol-table root-closure gensym/counter random/seed random/seed-initialize! random/rng! random/seed! random tree->json val->json ansi/disabled ansi-fg-reset ansi-bg-reset ansi-fg ansi-bg ansi-wrap ansi-black ansi-dark-red ansi-dark-green ansi-brown ansi-dark-blue ansi-purple ansi-teal ansi-dark-gray ansi-gray ansi-cyan ansi-rainbow split ansi-rainbow-bg reprint-line print fmt/format-arg/default fmt/find-non-digit-from-right fmt/parse-spec read/single fmt/debug fmt/number-format int->string/binary int->string/octal int->string/decimal int->string/hex int->string/HEX fmt/number-format-prefixex fmt/number-format-prefix fmt/add-padding string/pad-middle string/pad-end fmt/precision string/round fmt/truncate fmt/output fmt/format-arg fmt/valid-argument? fmt/expr/count fmt/expr fmt/args/map-fun/count fmt/args/map-fun fmt pfmt efmt pfmtln efmtln errorln string->byte-array br path/ext?! path/extension path/without-extension int->string/hex/conversion-arr int->string split/empty split/string read/int read/float string/length?! contains-any? contains-all? test-context test/reset test-list test-count test/add* nujel-start success-count error-count print-errors print-passes test/add display-results test-success test-failure test-bytecode test-default test-forked eval/forked test-run-real test-run test-run-bytecode test-run-forked input exit popen file/read file/write file/remove file/temp file/stat directory/read directory/remove directory/make path/change path/working-directory readline help file/compile file/eval file/eval/bytecode file/file? file/dir? directory/read-relative directory/read-recursive repl/executable-name repl/parse-args/bytecode-eval-n repl/parse-args/eval-next repl/parse-args/run-repl repl/options repl/option-map repl/exception-handler repl/history repl/prompt repl/wasm repl/readline repl repl/print-help repl/run-forked* repl/run-forked repl/parse-option repl/parse-options repl/parse-arg repl/parse-args repl/init/wasm repl/init/bin repl/init environment/variables");
    var indentKeys = makeKeywords("def defn let let* lambda define-macro defmacro when unless while for");

    function stateStack(indent, type, prev) { // represents a state stack object
        this.indent = indent;
        this.type = type;
        this.prev = prev;
        this.depth = prev ? (prev.depth|0) + (((type == '[') || (type == '(')) ? 1 : 0) : 0;
    }

    function pushStack(state, indent, type) {
        state.indentStack = new stateStack(indent, type, state.indentStack);
    }

    function popStack(state) {
        state.indentStack = state.indentStack.prev;
    }

    var binaryMatcher = new RegExp(/^(?:[-+]i|[-+][01]+#*(?:\/[01]+#*)?i|[-+]?[01]+#*(?:\/[01]+#*)?@[-+]?[01]+#*(?:\/[01]+#*)?|[-+]?[01]+#*(?:\/[01]+#*)?[-+](?:[01]+#*(?:\/[01]+#*)?)?i|[-+]?[01]+#*(?:\/[01]+#*)?)(?=[()\s;"]|$)/i);
    var octalMatcher = new RegExp(/^(?:[-+]i|[-+][0-7]+#*(?:\/[0-7]+#*)?i|[-+]?[0-7]+#*(?:\/[0-7]+#*)?@[-+]?[0-7]+#*(?:\/[0-7]+#*)?|[-+]?[0-7]+#*(?:\/[0-7]+#*)?[-+](?:[0-7]+#*(?:\/[0-7]+#*)?)?i|[-+]?[0-7]+#*(?:\/[0-7]+#*)?)(?=[()\s;"]|$)/i);
    var hexMatcher = new RegExp(/^(?:[-+]i|[-+][\da-f]+#*(?:\/[\da-f]+#*)?i|[-+]?[\da-f]+#*(?:\/[\da-f]+#*)?@[-+]?[\da-f]+#*(?:\/[\da-f]+#*)?|[-+]?[\da-f]+#*(?:\/[\da-f]+#*)?[-+](?:[\da-f]+#*(?:\/[\da-f]+#*)?)?i|[-+]?[\da-f]+#*(?:\/[\da-f]+#*)?)(?=[()\s;"]|$)/i);
    var decimalMatcher = new RegExp(/^(?:[-+]i|[-+](?:(?:(?:\d+#+\.?#*|\d+\.\d*#*|\.\d+#*|\d+)(?:[esfdl][-+]?\d+)?)|\d+#*\/\d+#*)i|[-+]?(?:(?:(?:\d+#+\.?#*|\d+\.\d*#*|\.\d+#*|\d+)(?:[esfdl][-+]?\d+)?)|\d+#*\/\d+#*)@[-+]?(?:(?:(?:\d+#+\.?#*|\d+\.\d*#*|\.\d+#*|\d+)(?:[esfdl][-+]?\d+)?)|\d+#*\/\d+#*)|[-+]?(?:(?:(?:\d+#+\.?#*|\d+\.\d*#*|\.\d+#*|\d+)(?:[esfdl][-+]?\d+)?)|\d+#*\/\d+#*)[-+](?:(?:(?:\d+#+\.?#*|\d+\.\d*#*|\.\d+#*|\d+)(?:[esfdl][-+]?\d+)?)|\d+#*\/\d+#*)?i|(?:(?:(?:\d+#+\.?#*|\d+\.\d*#*|\.\d+#*|\d+)(?:[esfdl][-+]?\d+)?)|\d+#*\/\d+#*))(?=[()\s;"]|$)/i);

    function isBinaryNumber (stream) {
        return stream.match(binaryMatcher);
    }

    function isOctalNumber (stream) {
        return stream.match(octalMatcher);
    }

    function isDecimalNumber (stream, backup) {
        if (backup === true) {
            stream.backUp(1);
        }
        return stream.match(decimalMatcher);
    }

    function isHexNumber (stream) {
        return stream.match(hexMatcher);
    }

    function processEscapedSequence(stream, options) {
        var next, escaped = false;
        while ((next = stream.next()) != null) {
            if (next == options.token && !escaped) {

                options.state.mode = false;
                break;
            }
            escaped = !escaped && next == "\\";
        }
    }

    return {
        startState: function () {
            return {
                indentStack: null,
                indentation: 0,
                mode: false,
                sExprComment: false,
                sExprQuote: false,
                depth: 0
            };
        },

        token: function (stream, state) {
            if (state.indentStack == null && stream.sol()) {
                // update indentation, but only if indentStack is empty
                state.indentation = stream.indentation();
            }

            // skip spaces
            if (stream.eatSpace()) {
                return null;
            }
            var returnType = null;

            switch(state.mode){
                case "string": // multi-line string parsing mode
                    processEscapedSequence(stream, {
                        token: "\"",
                        state: state
                    });
                    returnType = STRING; // continue on in scheme-string mode
                    break;
                case "symbol": // escape symbol
                    processEscapedSequence(stream, {
                        token: "|",
                        state: state
                    });
                    returnType = SYMBOL; // continue on in scheme-symbol mode
                    break;
                case "comment": // comment parsing mode
                    var next, maybeEnd = false;
                    while ((next = stream.next()) != null) {
                        if (next == "#" && maybeEnd) {

                            state.mode = false;
                            break;
                        }
                        maybeEnd = (next == "|");
                    }
                    returnType = COMMENT;
                    break;
                case "s-expr-comment": // s-expr commenting mode
                    state.mode = false;
                    if(stream.peek() == "(" || stream.peek() == "["){
                        // actually start scheme s-expr commenting mode
                        state.sExprComment = 0;
                    }else{
                        // if not we just comment the entire of the next token
                        stream.eatWhile(/[^\s\(\)\[\]]/); // eat symbol atom
                        returnType = COMMENT;
                        break;
                    }
                default: // default parsing mode
                    var ch = stream.next();

                    if (ch == "\"") {
                        state.mode = "string";
                        returnType = STRING;

                    } else if (ch == "'") {
                        if (stream.peek() == "(" || stream.peek() == "["){
                            if (typeof state.sExprQuote != "number") {
                                state.sExprQuote = 0;
                            } // else already in a quoted expression
                            returnType = ATOM;
                        } else {
                            stream.eatWhile(/[\w_\-!$%&*+\.\/:<=>?@\^~]/);
                            returnType = ATOM;
                        }
                    } else if (ch == '|') {
                        state.mode = "symbol";
                        returnType = SYMBOL;
                    } else if (ch == '#') {
                        if (stream.eat("|")) {                    // Multi-line comment
                            state.mode = "comment"; // toggle to comment mode
                            returnType = COMMENT;
                        } else if (stream.eat(/[tf]/i)) {            // #t/#f (atom)
                            returnType = ATOM;
                        } else if (stream.eat(';')) {                // S-Expr comment
                            state.mode = "s-expr-comment";
                            returnType = COMMENT;
                        } else {
                            var numTest = null, hasExactness = false, hasRadix = true;
                            if (stream.eat(/[ei]/i)) {
                                hasExactness = true;
                            } else {
                                stream.backUp(1);       // must be radix specifier
                            }
                            if (stream.match(/^#b/i)) {
                                numTest = isBinaryNumber;
                            } else if (stream.match(/^#o/i)) {
                                numTest = isOctalNumber;
                            } else if (stream.match(/^#x/i)) {
                                numTest = isHexNumber;
                            } else if (stream.match(/^#d/i)) {
                                numTest = isDecimalNumber;
                            } else if (stream.match(/^[-+0-9.]/, false)) {
                                hasRadix = false;
                                numTest = isDecimalNumber;
                            // re-consume the initial # if all matches failed
                            } else if (!hasExactness) {
                                stream.eat('#');
                            }
                            if (numTest != null) {
                                if (hasRadix && !hasExactness) {
                                    // consume optional exactness after radix
                                    stream.match(/^#[ei]/i);
                                }
                                if (numTest(stream))
                                    returnType = NUMBER;
                            }
                        }
                    } else if (/^[-+0-9.]/.test(ch) && isDecimalNumber(stream, true)) { // match non-prefixed number, must be decimal
                        returnType = NUMBER;
                    } else if (ch == ";") { // comment
                        stream.skipToEnd(); // rest of the line is a comment
                        returnType = COMMENT;
                    } else if (ch == "(" || ch == "[") {
                      var keyWord = ''; var indentTemp = stream.column(), letter;
                        /**
                        Either
                        (indent-word ..
                        (non-indent-word ..
                        (;something else, bracket, etc.
                        */

                        while ((letter = stream.eat(/[^\s\(\[\;\)\]]/)) != null) {
                            keyWord += letter;
                        }

                        if (keyWord.length > 0 && indentKeys.propertyIsEnumerable(keyWord)) { // indent-word

                            pushStack(state, indentTemp + INDENT_WORD_SKIP, ch);
                        } else { // non-indent word
                            // we continue eating the spaces
                            stream.eatSpace();
                            if (stream.eol() || stream.peek() == ";") {
                                // nothing significant after
                                // we restart indentation 1 space after
                                pushStack(state, indentTemp + 1, ch);
                            } else {
                                pushStack(state, indentTemp + stream.current().length, ch); // else we match
                            }
                        }
                        stream.backUp(stream.current().length - 1); // undo all the eating

                        if(typeof state.sExprComment == "number") state.sExprComment++;
                        if(typeof state.sExprQuote == "number") state.sExprQuote++;

                        const curDepth = state.indentStack.depth % 4;
                        returnType = `${BRACKET} depth-${curDepth} `;
                    } else if (ch == ")" || ch == "]") {
                        const curDepth = state.indentStack.depth % 4;
                        returnType = `${BRACKET} depth-${curDepth} `;
                        if (state.indentStack != null && state.indentStack.type == (ch == ")" ? "(" : "[")) {
                            popStack(state);

                            if(typeof state.sExprComment == "number"){
                                if(--state.sExprComment == 0){
                                    returnType = COMMENT; // final closing bracket
                                    state.sExprComment = false; // turn off s-expr commenting mode
                                }
                            }
                            if(typeof state.sExprQuote == "number"){
                                if(--state.sExprQuote == 0){
                                    returnType = ATOM; // final closing bracket
                                    state.sExprQuote = false; // turn off s-expr quote mode
                                }
                            }
                        }
                    } else {
                        stream.eatWhile(/[\w_\-!$%&*+\.\/:<=>?@\^~]/);

                        if (keywords && keywords.propertyIsEnumerable(stream.current())) {
                            returnType = BUILTIN;
                        } else returnType = "variable";
                    }
            }
            return (typeof state.sExprComment == "number") ? COMMENT : ((typeof state.sExprQuote == "number") ? ATOM : returnType);
        },

        indent: function (state) {
            if (state.indentStack == null) return state.indentation;
            return state.indentStack.indent;
        },

        fold: "brace-paren",
        closeBrackets: {pairs: "()[]{}\"\""},
        lineComment: ";;"
    };
});
});
