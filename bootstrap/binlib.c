/* This file is auto-generated, manual changes will be overwritten! */
unsigned char binlib_no_data[] = "#{##(\"nujel\" init/executable-name init/args tree/new init/options init/option-map init/parse-args/eval-next-module init/parse-args/eval-next init/parse-args/run-repl r (option) #@(name: anonymous) #{##(tiny-repl exit)\n"
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
"} :default (option) #{##(init/options option)\n"
"0E000E011B3701\n"
"} (option) #@(name: init/parse-option) #{##(init/option-map option :default #f)\n"
"0E000E012B0C0A00100D0E001A022B0C0A00060D1A030E01040101\n"
"} init/parse-option (options) #@(name: init/parse-options) #{##(options init/parse-option string->keyword cut for-each map split \"\" string->symbol)\n"
"0E0002002B022D200B00160E010E020E030E0002010402040104010900170E04\n"
"0E050E060E001A0704020E0804020E01040201\n"
"} init/parse-options (arg args) #@(name: init/parse-arg) #{##(init/parse-args/eval-next print/error eval-in root-closure do read arg init/parse-args/eval-next-module (e) #@(name: anonymous) #{##(print/error e exit)\n"
"0E000E0104010D0E020201040101\n"
"} string/cut module/main string->keyword args exit init/parse-options (e) #{##(print/error e exit)\n"
"0E000E0104010D0E020201040101\n"
"} file/eval-module init/parse-args/run-repl)\n"
"0E000B001F0E011900170E020E030E040E050E0604011404020D1C0500160900\n"
"850E070B003F1A081A091A0A171900320E0602002B023A200B00100E0B0E0602\n"
"0104020506090004240D0E0C0E0D0E0604010E0E1204020D0E0F020004011609\n"
"00440E0602002B022D200B00120E100E0B0E0602010402040109002A1B0B0025\n"
"1A111A091A12171900140E130E060E0E1204020D0E0F02000401160D1C051409\n"
"00042401\n"
"} init/parse-arg (args) #@(name: init/parse-args) #{##(args init/parse-arg init/parse-args init/parse-args/run-repl)\n"
"0E000B00170E010E00110E0004020D0E020E001204010900050E0301\n"
"} init/parse-args (args) #@(name: init/bin) #{##(print/error args init/executable-name init/parse-args module/main :repl)\n"
"0E0019001F0E011105020D0E030E011204010B000D0E041A0524040209000424\n"
"1601\n"
"} init/bin args #@(name: init) #{##(args init/args init/bin)\n"
"0E0005010D0E020E00040101\n"
"} init)\n"
"1A0007010D2407020D0E0324040107040D0E0324040107050D151C07060D1C07\n"
"070D1B07080D0E051A091A0A1A0B1A0C17370D0E051A0D1A0E1A0B1A0F17370D\n"
"0E051A101A111A0B1A1217370D0E051A131A141A0B1A1517370D0E051A161A17\n"
"1A0B1A1817370D0E051A191A1A1A0B1A1B17370D0E051A1C1A1D1A0B1A1E1737\n"
"0D1A1F1A201A211707220D1A231A241A251707260D1A271A281A2917072A0D1A\n"
"2B1A2C1A2D17072E0D1A2F1A301A311707320D1A331A341A3517073616073601\n"
"}#{##((source-path destination-path) #@(documentation: \"Copy a file from SOURCE-PATH to DESTINATION-PATH\" name: file/copy) #{##(spit destination-path slurp source-path)\n"
"0E000E010E020E030401040201\n"
"} file/copy (path) #@(documentation: \"Read in a file using the Nujel reader\" name: file/read) #{##(read file/load path)\n"
"0E000E010E020401040101\n"
"} file/read (path) #@(documentation: \"Read a single value from a file\" name: file/read/single) #{##(file/read path)\n"
"0E000E0104011101\n"
"} file/read/single (path environment) #@(documentation: \"Evaluate a Nujel source file in the current context\" name: file/eval) #{##(eval-in environment root-closure #f do read file/read path)\n"
"0E000E010C0A000D0D0E020C0A00060D1A031A040E050E060E07040104011404\n"
"0201\n"
"} file/eval (path args) #@(documentation: \"Evaluate a Nujel source file in the current context\" name: file/eval-module) #{##(module/resolve-string cat path/without-extension path path/working-directory mod-name module/load mod :environment type-of resolve exports \"Couldn't load \" \" as a module since it has no exports\" current-lambda tree/has? :main args)\n"
"0E000E010E020E03040104010E040400040207050D0E060E05040107070D1A08\n"
"0E090E070401200B0007240900060E07010D0E0A1A0B0E070402070B0D0E0B0B\n"
"0007240900160E011A0C0E031A0D040324240E0E04002E042F0D0E0F0E0B1A10\n"
"04020B000F0E0B1A102B0E1104010900042401\n"
"} file/eval-module (path environment) #@(documentation: \"Compile a Nujel source file into optimized object code\" name: file/compile) #{##(do read file/read path source compile* environment #f object-code file/write string/write \"\" cat path/without-extension \".no\")\n"
"1A000E010E020E03040104011407040D0E050E040E060C0A00100D15240D1316\n"
"0C0A00060D1A07040207080D0E090E080B000C0E0A0E0804010900051A0B0E0C\n"
"0E0D0E0304011A0E040204020D0E080101\n"
"} file/compile (path environment base-dir) #@(documentation: \"Compile a Nujel source file into optimized object code\" name: file/compile/module) #{##(string->keyword path/without-extension string/cut path :length base-dir module-name defmodule/defer def *module* append read file/read source compile* environment #f object-code file/write string/write \"\" cat \".no\")\n"
"0E000E010E020E031A040E05040104020401040107060D1A070E061A081A090E\n"
"06241414140E0A0E0B0E0C0E0304010401240402141414070D0D0E0E0E0D0E0F\n"
"0C0A00100D15240D13160C0A00060D1A10040207110D0E120E110B000C0E130E\n"
"1104010900051A140E150E010E0304011A16040204020D0E110101\n"
"} file/compile/module (#nil) #@(name: file/compile/argv) #{##(last-pair init/args path index-of \"_modules/\" module file/compile/module string/cut file/compile exit)\n"
"0E000E0104011107020D0E030E021A04040207050D0E050200210B001A0E060E\n"
"02240E070E0202000E05020925040304030900090E080E0204010D0E09020004\n"
"0101\n"
"} file/compile/argv (tests module-name) #@(name: file/test/module/run) #{##(module/import module/load :test current-closure run-test! tests ΓεnΣym-16 expr compile* do require module-name append)\n"
"0E000E011A020E03040004021A04040207040D150E0507060D240900380D0E06\n"
"1107070D0E040E080E072C0E03040004020E0304001D1A091A0A0E0B2414140E\n"
"0C0E071212240402141404020D0E061205060E060AFFC90D241601\n"
"} file/test/module/run (form) #@(name: file/test/valid-test-form?) #{##(form deftest)\n"
"0E00111A012001\n"
"} file/test/valid-test-form? (path base-dir) #@(documentation: \"Test a module by running all contained tests\" name: file/test/module) #{##(string/cut path :length base-dir rel-path string->keyword path/without-extension module-name module/import module/load :ansi current-closure blue :test init! finish! file/test/module/run filter read file/read file/test/valid-test-form?)\n"
"0E000E011A020E030401040207040D0E0402002B022F200B00100E000E040201\n"
"04020504090004240D0E050E060E040401040107070D0E080E091A0A0E0B0400\n"
"04021A0C0402070C0D0E080E091A0D0E0B040004021A0E0402070E0D0E080E09\n"
"1A0D0E0B040004021A0F0402070F0D0E0E04000D0E100E110E120E130E010401\n"
"04010E1404020E0704020D0E0F0E0C0E070401040101\n"
"} file/test/module (base-dir) #@(documentation: \"Compile a Nujel source file into optimized object code\" name: file/test/directory) #{##(sum map filter sort flatten directory/read-recursive base-dir path/ext?! \"nuj\" (path) #@(name: anonymous) #{##(file/test/module path base-dir)\n"
"0E000E010E02040201\n"
"})\n"
"0E000E010E020E030E040E050E060401040104010E071A08040104021A091A0A\n"
"1A0B170402040101\n"
"} file/test/directory (filename) #@(name: file/file?) #{##(file/stat filename :regular-file?)\n"
"0E000E0104011A022B01\n"
"} file/file? (filename) #@(name: file/dir?) #{##(file/stat filename :directory?)\n"
"0E000E0104011A022B01\n"
"} file/dir? (path) #@(name: directory/read-relative) #{##(map directory/read path (a) #@(name: anonymous) #{##(cat path \"/\" a)\n"
"0E000E011A020E03040301\n"
"})\n"
"0E000E010E0204011A031A041A0517040201\n"
"} directory/read-relative (A) #@(name: directory/read-recursive/fn) #{##(file/dir? A directory/read-recursive)\n"
"0E000E0104010B000C0E020E0104010900050E0101\n"
"} directory/read-recursive/fn (path) #@(name: directory/read-recursive) #{##(flatten filter map directory/read-relative path directory/read-recursive/fn identity)\n"
"0E000E010E020E030E0404010E0504020E060402040101\n"
"} directory/read-recursive (filename) #@(name: load) #{##((err) #@(name: anonymous) #{##(print/error err)\n"
"0E000E0104010D1C01\n"
"} file/eval filename println cat \"Loaded \")\n"
"1A001A011A02171900190E030E0404010D0E050E061A070E04040204010D1B16\n"
"01\n"
"} load (pathname) #@(documentation: \"Read the entirety of PATHNAME and return it as a string if possible, otherwise return #nil.\" name: slurp/buffer) #{##(file/open-input* pathname fh (#nil) #@(name: anonymous) #{##(file/close* fh)\n"
"0E000E0104010D240101\n"
"} file/seek* file/tell* size buffer/allocate buf file/read* file/close*)\n"
"0E000E01040107020D0E020B00072409000524010D1A031A041A05171900410E\n"
"060E020200020204030D0E070E02040107080D0E060E020200020004030D0E09\n"
"0E080401070A0D0E0B0E020E0A0E0804030D0E0C0E0204010D0E0A011601\n"
"} slurp/buffer file/read/buffer (pathname) #@(documentation: \"Read the entirety of PATHNAME and return it as a string if possible, otherwise return #nil.\" name: slurp) #{##(buffer->string slurp/buffer pathname)\n"
"0E000E010E020401040101\n"
"} slurp (pathname content) #@(documentation: \"Read the entirety of PATHNAME and return it as a string if possible, otherwise return #nil.\" name: spit) #{##(file/open-output* pathname :replace fh (#nil) #@(name: anonymous) #{##(file/close* fh)\n"
"0E000E0104010D1C0101\n"
"} file/write* content file/close*)\n"
"0E000E011A02040207030D0E030B0007240900051C010D1A041A051A06171900\n"
"130E070E030E0804020D0E090E030401160D1B0101\n"
"} spit (content pathname) #@(documentation: \"Writes CONTENT into PATHNAME\" name: file/write) #{##(spit pathname content)\n"
"0E000E010E02040201\n"
"} file/write make-input-port stdin* stdin (#nil) #@(name: current-input-port) #{##(stdin)\n"
"0E0001\n"
"} current-input-port (nport) #@(name: current-input-port!) #{##(nport stdin)\n"
"0E00050101\n"
"} current-input-port! make-output-port stdout* stdout (#nil) #@(name: current-output-port) #{##(stdout)\n"
"0E0001\n"
"} current-output-port (nport) #@(name: current-output-port!) #{##(nport stdout)\n"
"0E00050101\n"
"} current-output-port! stderr* stderr (#nil) #@(name: current-error-port) #{##(stderr)\n"
"0E0001\n"
"} current-error-port (nport) #@(name: current-error-port!) #{##(nport stderr)\n"
"0E00050101\n"
"} current-error-port! (port) #@(documentation: \"Print a single line feed character\" name: newline) #{##(port stdout #f block-write \"\\r\\n\")\n"
"0E000C0A000D0D0E010C0A00060D1A021A031A0404020D2401\n"
"} newline (v port) #@(documentation: \"Display V on the standard output port\" name: print) #{##(write/raw v port stdout #f)\n"
"0E000E010E020C0A000D0D0E030C0A00060D1A041B04030D2401\n"
"} print (v port) #@(documentation: \"Prints v on the standard error port\" name: error) #{##(print v stderr)\n"
"0E000E010E0204020D2401\n"
"} error (port buf) #@(documentation: \"Reads in a line of user input and returns it\" name: read-line/raw) #{##(i c buffer/u8* buf view :length! 128 :length port char-read :end-of-file)\n"
"020007000D020007010D0E020E03040107040D240900650D240900110D1A051A\n"
"061A070E0304012504010E001A070E030401210AFFE90D0E081A09040105010D\n"
"0E011A0A200B00130E002A0B0007240900050E0001090004240D0E01020A200B\n"
"00090E0001090004240D0E040E000E01370D02010E002505001B0AFF9D01\n"
"} read-line/raw (#nil) #@(documentation: \"Reads in a line of user input and returns it\" name: read-line) #{##(buffer/allocate 128 buf read-line/raw stdin len buffer->string)\n"
"0E001A01040107020D0E030E040E02040207050D0E050C0B000C0D0E060E020E\n"
"05040201\n"
"} read-line input (prompt) #@(documentation: \"Read a line of input in a user friendly way after writing PROMPT\" name: readline) #{##(stdout block-write prompt \"\" #f flush-output read-line)\n"
"0E001A010E020C0A000D0D1A030C0A00060D1A0404020D0E001A0504010D0E06\n"
"040001\n"
"} readline (cmd) #@(documentation: \"Run CMD using popen and return the trimmed stdout\" name: popen/trim) #{##(trim popen cmd)\n"
"0E000E010E02040112040101\n"
"} popen/trim path/working-directory +root-working-dir+ *module-path* (name) #@(name: module/loader/filesystem) #{##(keyword->string name name-string System/OS Windows string/cut fmt-arg-0 cat \".nuj\" module-path file/read source module def *module* *module-path* path/dirname do read expr compile* current-closure mod)\n"
"0E000E01040107020D0E0202002B022F200B00072409000524010D0E031A0420\n"
"0B00100E050E02020104020502090004240D150E0207060D0E070E061A080402\n"
"1607090D0E0A0E090401070B0D0E0B0B00072409000524010D1A0C1A0D1A0E0E\n"
"01241414141A0D1A0F0E100E090401241414140E110E120E0B0401142E040713\n"
"0D0E140E130E15040004020E1504001D07160D0E160101\n"
"} module/loader/filesystem module/add-loader)\n"
"1A001A011A021707030D1A041A051A061707070D1A081A091A0A17070B0D1A0C\n"
"1A0D1A0E17070F0D1A101A111A121707130D1A141A151A161707170D1A181A19\n"
"1A1A17071B0D1A1C1A1D1A1E17071F0D1A201A211A221707230D1A241A251A26\n"
"1707270D1A281A291A2A17072B0D1A2C1A2D1A2E17072F0D1A301A311A321707\n"
"330D1A341A351A361707370D1A381A391A3A17073B0D151A3C1A3D1A3E17073F\n"
"0D1A401A411A421707431607430D1A441A451A461707470D1A481A491A4A1707\n"
"4B0D0E4B074C0D1A4D1A4E1A4F1707500D0E5007070D1A511A521A531707540D\n"
"1A551A561A571707580D0E590E5A0401075B0D1A5C1A5D1A5E17075F0D1A601A\n"
"611A621707630D0E640E65040107660D1A671A681A6917076A0D1A6B1A6C1A6D\n"
"17076E0D0E640E6F040107700D1A711A721A731707740D1A751A761A77170778\n"
"0D1A791A7A1A7B17077C0D1A7D1A7E1A7F1707800D1A811A821A831707840D1A\n"
"851A861A871707880D1A891A8A1A8B17078C0D0E8C078D0D1A8E1A8F1A901707\n"
"910D1A921A931A941707950D0E96040007970D0E96040007980D1A991A9A1A9B\n"
"17079C0D0E9D0E9C040101\n"
"}#{##(ctx (error) #@(name: exception-handler) #{##(print/error error)\n"
"0E000E01040101\n"
"} exception-handler exports cmd/raw (line) #@(name: cmd/raw export: #t) #{##((err) #@(name: anonymous) #{##(err :unmatched-opening-bracket cmd/raw ctx cat line readline \"... \")\n"
"0E00111A01200B00180E020E030E040E050E061A070401040204020900060E00\n"
"2F01\n"
"} read line expr equal? (#nil) print \"\\r\" exception-handler eval-in ctx do result display newline)\n"
"1A001A011A02171900450E030E04040107050D0E061A070E0504020B000F0E08\n"
"1A0904010D2401090004240D0E0A19001D0E0B0E0C0E0D0E05140402070E0D0E\n"
"0F0E0E04010D0E100400161601\n"
"} (#nil) #@(name: cmd) #{##(\"\" buf line cat readline not= trim \"[/cmd]\" exception-handler do read expr eval-in ctx result display newline)\n"
"1A0007010D1A0007020D240900150D0E030E010E02040205010D0E0404000502\n"
"0E050E060E0204011A0704020AFFE20D0E081900260E090E0A0E01040114070B\n"
"0D0E0C0E0D0E0B0402070E0D0E0F0E0E04010D0E1004001601\n"
"} cmd (#nil) #@(name: read-cmd) #{##(readline \"> \" line nil? println \"Adios, cowboy...\" exit trim \"[cmd]\" cmd cmd/raw)\n"
"0E001A01040107020D0E030E0204010B00130E041A0504010D0E060200040109\n"
"0004240D0E070E0204011A08200B000A0E0904000900090E0A0E02040101\n"
"} read-cmd (#nil) #@(name: tiny-repl) #{##(println \"Nujel TinyREPL is ready for service!\" read-cmd)\n"
"0E001A0104010D240900080D0E0204001B0AFFFA01\n"
"} tiny-repl)\n"
"1515240D131607000D1A011A021A031707040D0E051A061A071A081A09170706\n"
"370D1A0A1A0B1A0C17070D0D1A0E1A0F1A101707110D1A121A131A1417071516\n"
"071501\n"
"}";