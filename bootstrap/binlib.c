/* This file is auto-generated, manual changes will be overwritten! */
unsigned char binlib_no_data[] = "#{##(\"nujel\" init/executable-name init/args tree/new init/options init/option-map init/parse-args/eval-next-module init/parse-args/eval-next init/parse-args/run-repl tree/set! r anonymous (option) #@() #{##(tiny-repl exit)\n"
"0E0004000D0E010200040101\n"
"} m (option) #{##(init/parse-args/eval-next-module)\n"
"1B050001\n"
"} h (option) #{##(module/main :help exit)\n"
"0E001A012404020D0E020200040101\n"
"} no-color (option) #{##(module/import module/load :ansi current-closure disable!)\n"
"0E000E011A020E03040004021A04040207040D1B050401\n"
"} color (option) #{##(module/import module/load :ansi current-closure disable!)\n"
"0E000E011A020E03040004021A04040207040D1C050401\n"
"} x (option) #{##(init/parse-args/eval-next init/parse-args/run-repl)\n"
"1B05000D1C050101\n"
"} :default (option) #{##(tree/set! init/options option)\n"
"0E000E010E021B040301\n"
"} init/parse-option (option) #{##(init/option-map option :default #f)\n"
"0E000E012B0C0A00100D0E001A022B0C0A00060D1A030E01040101\n"
"} init/parse-options (options) #{##(options init/parse-option string->keyword cut for-each map split \"\" string->symbol)\n"
"0E0002002B022D200B00160E010E020E030E0002010402040104010900170E04\n"
"0E050E060E001A0704020E0804020E01040201\n"
"} init/parse-arg (arg args) #{##(init/parse-args/eval-next print/error eval-in root-closure do read arg init/parse-args/eval-next-module anonymous (e) #@() #{##(print/error e exit)\n"
"0E000E0104010D0E020201040101\n"
"} string/cut module/main string->keyword args exit init/parse-options (e) #{##(print/error e exit)\n"
"0E000E0104010D0E020201040101\n"
"} file/eval-module init/parse-args/run-repl)\n"
"0E000B001F0E011900170E020E030E040E050E0604011404020D1C0500160900\n"
"890E070B00411A081A091A0A1A0B171900320E0602002B023A200B00100E0C0E\n"
"06020104020506090004240D0E0D0E0E0E0604010E0F1204020D0E1002000401\n"
"160900460E0602002B022D200B00120E110E0C0E0602010402040109002C1B0B\n"
"00271A081A121A0A1A13171900140E140E060E0F1204020D0E1002000401160D\n"
"1C05150900042401\n"
"} init/parse-args (args) #{##(args init/parse-arg init/parse-args init/parse-args/run-repl)\n"
"0E000B00170E010E00110E0004020D0E020E001204010900050E0301\n"
"} init/bin (args) #{##(print/error args init/executable-name init/parse-args module/main :repl)\n"
"0E0019001F0E011105020D0E030E011204010B000D0E041A0524040209000424\n"
"1601\n"
"} init args #{##(args init/args System/Architecture wasm init/wasm init/bin)\n"
"0E0005010D0E021A03200B000C0E040E0004010900090E050E00040101\n"
"})\n"
"1A0007010D2407020D0E0324040107040D0E0324040107050D151C07060D1C07\n"
"070D1B07080D0E090E051A0A1A0B1A0C1A0D1A0E1704030D0E090E051A0F1A0B\n"
"1A101A0D1A111704030D0E090E051A121A0B1A131A0D1A141704030D0E090E05\n"
"1A151A0B1A161A0D1A171704030D0E090E051A181A0B1A191A0D1A1A1704030D\n"
"0E090E051A1B1A0B1A1C1A0D1A1D1704030D0E090E051A1E1A0B1A1F1A0D1A20\n"
"1704030D1A211A221A0D1A231707210D1A241A251A0D1A261707240D1A271A28\n"
"1A0D1A291707270D1A2A1A2B1A0D1A2C17072A0D1A2D1A2E1A0D1A2F17072D0D\n"
"1A301A311A0D1A3217073016073001\n"
"}#{##(file/copy (source-path destination-path) #@(documentation: \"Copy a file from SOURCE-PATH to DESTINATION-PATH\") #{##(spit destination-path slurp source-path)\n"
"0E000E010E020E030401040201\n"
"} file/read (path) #@(documentation: \"Read in a file using the Nujel reader\") #{##(read file/load path)\n"
"0E000E010E020401040101\n"
"} file/read/single (path) #@(documentation: \"Read a single value from a file\") #{##(file/read path)\n"
"0E000E0104011101\n"
"} file/eval (path environment) #@(documentation: \"Evaluate a Nujel source file in the current context\") #{##(eval-in environment root-closure #f do read file/read path)\n"
"0E000E010C0A000D0D0E020C0A00060D1A031A040E050E060E07040104011404\n"
"0201\n"
"} file/eval-module (path args) #{##(module/resolve-string cat path/without-extension path path/working-directory mod-name module/load mod :environment type-of resolve exports throw \"Couldn't load \" \" as a module since it has no exports\" current-lambda tree/has? :main args)\n"
"0E000E010E020E03040104010E040400040207050D0E060E05040107070D1A08\n"
"0E090E070401200B0007240900060E07010D0E0A1A0B0E070402070B0D0E0B0B\n"
"0007240900190E0C0E011A0D0E031A0E040324240E0F04002E0404010D0E100E\n"
"0B1A1104020B000F0E0B1A112B0E1204010900042401\n"
"} file/compile (path environment) #@(documentation: \"Compile a Nujel source file into optimized object code\") #{##(do read file/read path source compile* environment #f object-code file/write string/write \"\" cat path/without-extension \".no\")\n"
"1A000E010E020E03040104011407040D0E050E040E060C0A00100D15240D1316\n"
"0C0A00060D1A07040207080D0E090E080B000C0E0A0E0804010900051A0B0E0C\n"
"0E0D0E0304011A0E040204020D0E080101\n"
"} file/compile/module (path environment base-dir) #{##(string->keyword path/without-extension string/cut path length base-dir module-name defmodule/defer def *module* append read file/read source compile* environment #f object-code file/write string/write \"\" cat \".no\")\n"
"0E000E010E020E030E040E05040104020401040107060D1A070E061A081A090E\n"
"06241414140E0A0E0B0E0C0E0304010401240402141414070D0D0E0E0E0D0E0F\n"
"0C0A00100D15240D13160C0A00060D1A10040207110D0E120E110B000C0E130E\n"
"1104010900051A140E150E010E0304011A16040204020D0E110101\n"
"} file/compile/argv (#nil) #@() #{##(last-pair init/args path index-of \"_modules/\" module file/compile/module string/cut file/compile exit)\n"
"0E000E0104011107020D0E030E021A04040207050D0E050200210B001A0E060E\n"
"02240E070E0202000E05020925040304030900090E080E0204010D0E09020004\n"
"0101\n"
"} file/test/module/run (tests module-name) #{##(module/import module/load :test current-closure run-test! tests ΓεnΣym-16 expr compile* do require module-name append)\n"
"0E000E011A020E03040004021A04040207040D150E0507060D240900380D0E06\n"
"1107070D0E040E080E072C0E03040004020E0304001D1A091A0A0E0B2414140E\n"
"0C0E071212240402141404020D0E061205060E060AFFC90D241601\n"
"} file/test/valid-test-form? (form) #{##(form deftest)\n"
"0E00111A012001\n"
"} file/test/module (path base-dir) #@(documentation: \"Test a module by running all contained tests\") #{##(string/cut path length base-dir rel-path string->keyword path/without-extension module-name module/import module/load :ansi current-closure blue :test init! finish! file/test/module/run filter read file/read file/test/valid-test-form?)\n"
"0E000E010E020E030401040207040D0E0402002B022F200B00100E000E040201\n"
"04020504090004240D0E050E060E040401040107070D0E080E091A0A0E0B0400\n"
"04021A0C0402070C0D0E080E091A0D0E0B040004021A0E0402070E0D0E080E09\n"
"1A0D0E0B040004021A0F0402070F0D0E0E04000D0E100E110E120E130E010401\n"
"04010E1404020E0704020D0E0F0E0C0E070401040101\n"
"} file/test/directory (base-dir) #{##(sum map filter sort flatten directory/read-recursive base-dir path/ext?! \"nuj\" anonymous (path) #@() #{##(file/test/module path base-dir)\n"
"0E000E010E02040201\n"
"})\n"
"0E000E010E020E030E040E050E060401040104010E071A08040104021A091A0A\n"
"1A0B1A0C170402040101\n"
"} file/file? (filename) #{##(file/stat filename :regular-file?)\n"
"0E000E0104011A022B01\n"
"} file/dir? (filename) #{##(file/stat filename :directory?)\n"
"0E000E0104011A022B01\n"
"} directory/read-relative (path) #{##(map directory/read path anonymous (a) #@() #{##(cat path \"/\" a)\n"
"0E000E011A020E03040301\n"
"})\n"
"0E000E010E0204011A031A041A051A0617040201\n"
"} directory/read-recursive/fn (A) #{##(file/dir? A directory/read-recursive)\n"
"0E000E0104010B000C0E020E0104010900050E0101\n"
"} directory/read-recursive (path) #{##(flatten filter map directory/read-relative path directory/read-recursive/fn identity)\n"
"0E000E010E020E030E0404010E0504020E060402040101\n"
"} load (filename) #{##(anonymous (err) #@() #{##(print/error err)\n"
"0E000E0104010D1C01\n"
"} file/eval filename println cat \"Loaded \")\n"
"1A001A011A021A03171900190E040E0504010D0E060E071A080E05040204010D\n"
"1B1601\n"
"} slurp/buffer (pathname) #@(documentation: \"Read the entirety of PATHNAME and return it as a string if possible, otherwise return #nil.\") #{##(file/open-input* pathname fh anonymous (#nil) #@() #{##(file/close* fh)\n"
"0E000E0104010D240101\n"
"} file/seek* file/tell* size buffer/allocate buf file/read* file/close*)\n"
"0E000E01040107020D0E020B00072409000524010D1A031A041A051A06171900\n"
"410E070E020200020204030D0E080E02040107090D0E070E020200020004030D\n"
"0E0A0E090401070B0D0E0C0E020E0B0E0904030D0E0D0E0204010D0E0B011601\n"
"} file/read/buffer slurp (pathname) #{##(buffer->string slurp/buffer pathname)\n"
"0E000E010E020401040101\n"
"} spit (pathname content) #{##(file/open-output* pathname :replace fh anonymous (#nil) #@() #{##(file/close* fh)\n"
"0E000E0104010D1C0101\n"
"} file/write* content file/close*)\n"
"0E000E011A02040207030D0E030B0007240900051C010D1A041A051A061A0717\n"
"1900130E080E030E0904020D0E0A0E030401160D1B0101\n"
"} file/write (content pathname) #@(documentation: \"Writes CONTENT into PATHNAME\") #{##(spit pathname content)\n"
"0E000E010E02040201\n"
"} make-input-port stdin* stdin current-input-port (#nil) #{##(stdin)\n"
"0E0001\n"
"} current-input-port! (nport) #{##(nport stdin)\n"
"0E00050101\n"
"} make-output-port stdout* stdout current-output-port (#nil) #{##(stdout)\n"
"0E0001\n"
"} current-output-port! (nport) #{##(nport stdout)\n"
"0E00050101\n"
"} stderr* stderr current-error-port (#nil) #{##(stderr)\n"
"0E0001\n"
"} current-error-port! (nport) #{##(nport stderr)\n"
"0E00050101\n"
"} newline (port) #@(documentation: \"Print a single line feed character\") #{##(port stdout #f block-write \"\\r\\n\")\n"
"0E000C0A000D0D0E010C0A00060D1A021A031A0404020D2401\n"
"} print (v port) #@(documentation: \"Display V on the standard output port\") #{##(write/raw v port stdout #f)\n"
"0E000E010E020C0A000D0D0E030C0A00060D1A041B04030D0E010101\n"
"} error (v port) #@(documentation: \"Prints v on the standard error port\") #{##(print v stderr)\n"
"0E000E010E02040201\n"
"} read-line/raw (port buf) #@(documentation: \"Reads in a line of user input and returns it\") #{##(i c buffer/u8* buf view buffer/length! 128 buffer/length port char-read :end-of-file buffer/set!)\n"
"020007000D020007010D0E020E03040107040D240900680D240900110D0E051A\n"
"060E070E0304012504010E000E070E030401210AFFE90D0E081A09040105010D\n"
"0E011A0A200B00130E002A0B0007240900050E0001090004240D0E01020A200B\n"
"00090E0001090004240D0E0B0E040E000E0104030D02010E002505001B0AFF9A\n"
"01\n"
"} read-line (#nil) #{##(buffer/allocate 128 buf read-line/raw stdin len buffer->string)\n"
"0E001A01040107020D0E030E040E02040207050D0E050C0B000C0D0E060E020E\n"
"05040201\n"
"} input readline (prompt) #@(documentation: \"Read a line of input in a user friendly way after writing PROMPT\") #{##(stdout block-write prompt \"\" #f flush-output read-line)\n"
"0E001A010E020C0A000D0D1A030C0A00060D1A0404020D0E001A0504010D0E06\n"
"040001\n"
"} popen/trim (cmd) #@(documentation: \"Run CMD using popen and return the trimmed stdout\") #{##(trim popen cmd)\n"
"0E000E010E02040112040101\n"
"} path/working-directory +root-working-dir+ *module-path* module/loader/filesystem (name) #{##(keyword->string name name-string System/OS Windows string/cut fmt-arg-0 cat \".nuj\" module-path file/read source module def *module* *module-path* path/dirname do read expr compile* current-closure mod)\n"
"0E000E01040107020D0E0202002B022F200B00072409000524010D0E031A0420\n"
"0B00100E050E02020104020502090004240D150E0207060D0E070E061A080402\n"
"1607090D0E0A0E090401070B0D0E0B0B00072409000524010D1A0C1A0D1A0E0E\n"
"01241414141A0D1A0F0E100E090401241414140E110E120E0B0401142E040713\n"
"0D0E140E130E15040004020E1504001D07160D0E160101\n"
"} module/add-loader)\n"
"1A001A011A021A031707000D1A041A051A061A071707040D1A081A091A0A1A0B\n"
"1707080D1A0C1A0D1A0E1A0F17070C0D1A101A111A0E1A121707100D1A131A14\n"
"1A151A161707130D1A171A181A151A191707170D1A1A1A1B1A1C1A1D17071A0D\n"
"1A1E1A1F1A1C1A2017071E0D1A211A221A1C1A231707210D1A241A251A261A27\n"
"1707240D1A281A291A151A2A1707280D1A2B1A2C1A1C1A2D17072B0D1A2E1A2F\n"
"1A1C1A3017072E0D1A311A321A1C1A331707310D151A341A351A1C1A36170734\n"
"0D1A371A381A1C1A391707371607370D1A3A1A3B1A1C1A3C17073A0D1A3D1A3E\n"
"1A3F1A4017073D0D0E3D07410D1A421A431A3F1A441707420D0E4207040D1A45\n"
"1A461A3F1A471707450D1A481A491A4A1A4B1707480D0E4C0E4D0401074E0D1A\n"
"4F1A501A1C1A5117074F0D1A521A531A1C1A541707520D0E550E56040107570D\n"
"1A581A591A1C1A5A1707580D1A5B1A5C1A1C1A5D17075B0D0E550E5E0401075F\n"
"0D1A601A611A1C1A621707600D1A631A641A1C1A651707630D1A661A671A681A\n"
"691707660D1A6A1A6B1A6C1A6D17076A0D1A6E1A6F1A701A7117076E0D1A721A\n"
"731A741A751707720D1A761A771A741A781707760D0E7607790D1A7A1A7B1A7C\n"
"1A7D17077A0D1A7E1A7F1A801A8117077E0D0E82040007830D0E82040007840D\n"
"1A851A861A1C1A871707850D0E880E85040101\n"
"}#{##(*1 *2 *3 line-history ctx exception-handler (error) #@() #{##(print/error error)\n"
"0E000E01040101\n"
"} push-result (result) #{##(*2 *3 *1 result)\n"
"0E0005010D0E0205000D0E0305020D0E030101\n"
"} tree/set! exports cmd/raw (line) #@(export: #t) #{##(anonymous (err) #@() #{##(err :unmatched-opening-bracket cmd/raw ctx cat line readline \"... \" throw)\n"
"0E00111A01200B00180E020E030E040E050E061A070401040204020900090E08\n"
"0E00040101\n"
"} read line expr equal? (#nil) print \"\\r\" exception-handler eval-in ctx do result push-result println nil? \"\" string/display)\n"
"1A001A011A021A03171900590E040E05040107060D0E071A080E0604020B000F\n"
"0E091A0A04010D2401090004240D0E0B1900310E0C0E0D0E0E0E06140402070F\n"
"0D0E100E0F04010D0E110E120E0F04010B00081A130900090E140E0F04010401\n"
"161601\n"
"} cmd (#nil) #{##(\"\" buf line cat readline not= trim \"[/cmd]\" do read expr eval-in ctx result push-result println nil? string/display)\n"
"1A0007010D1A0007020D240900150D0E030E010E02040205010D0E0404000502\n"
"0E050E060E0204011A0704020AFFE20D0E080E090E01040114070A0D0E0B0E0C\n"
"0E0A0402070D0D0E0E0E0D04010D0E0F0E100E0D04010B00081A000900090E11\n"
"0E0D0401040101\n"
"} prompt (#nil) #@(documentation: \">\") #{##(\"> \")\n"
"1A0001\n"
"} read-cmd (#nil) #{##(readline prompt line line-history nil? println \"Adios, cowboy...\" exit trim \"[cmd]\" cmd cmd/raw)\n"
"0E000E010400040107020D0E020E031405030D0E040E0204010B00130E051A06\n"
"04010D0E0702000401090004240D0E080E0204011A09200B000A0E0A04000900\n"
"090E0B0E02040101\n"
"} tiny-repl (#nil) #{##(println cat \"Nujel TinyREPL is ready for service!\" exception-handler read-cmd)\n"
"0E000E011A02040104010D2409000E0D0E031900080E040400161B0AFFF401\n"
"})\n"
"152407000D2407010D2407020D2407030D15240D131607040D1A051A061A071A\n"
"081707050D1A091A0A1A071A0B1707090D0E0C0E0D1A0E1A0E1A0F1A101A1117\n"
"070E04030D1A121A131A071A141707120D1A151A161A171A181707150D1A191A\n"
"1A1A071A1B1707190D1A1C1A1D1A071A1E17071C16071C01\n"
"}#{##(repl/wasm (line) #@(documentation: \"Evaluate LINE in the wasm context\") #{##(string/write print/error eval-in root-closure do read line)\n"
"0E000E011900130E020E030E040E050E06040114040216040101\n"
"} init/wasm (args) #@() #{##(repl/welcome)\n"
"0E00040001\n"
"})\n"
"1A001A011A021A031707000D1A041A051A061A0717070401\n"
"}";