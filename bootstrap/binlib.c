/* This file is auto-generated, manual changes will be overwritten! */
unsigned char binlib_no_data[] = "#{##(\"nujel\" init/executable-name init/args tree/new init/options init/option-map init/parse-args/eval-next-module init/parse-args/eval-next init/parse-args/run-repl init/parse-args/ignore-next r (option) #@(name: anonymous) #{##(tiny-repl exit)\n"
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
"} init/parse-option (options) #@(name: init/parse-options) #{##(options init/parse-option :keyword cut for-each map split \"\" :symbol)\n"
"0E0002002B022D200B00160E011A020E030E0002010402040104010900170E04\n"
"0E050E060E001A0704021A0804020E01040201\n"
"} init/parse-options (arg args) #@(name: init/parse-arg) #{##(init/parse-args/ignore-next init/parse-args/eval-next print/error eval-in root-closure do read arg init/parse-args/eval-next-module (e) #@(name: anonymous) #{##(print/error e exit)\n"
"0E000E0104010D0E020201040101\n"
"} :cut module/main :keyword args exit init/parse-options (e) #{##(print/error e exit)\n"
"0E000E0104010D0E020201040101\n"
"} file/eval-module init/parse-args/run-repl)\n"
"0E000B00091C05000900A60E010B001F0E021900170E030E040E050E060E0704\n"
"011404020D1C0501160900850E080B003F1A091A0A1A0B171900320E0702002B\n"
"023A200B00101A0C0E07020104020507090004240D0E0D1A0E0E0704010E0F12\n"
"04020D0E1002000401160900440E0702002B022D200B00120E111A0C0E070201\n"
"0402040109002A1B0B00251A121A0A1A13171900140E140E070E0F1204020D0E\n"
"1002000401160D1C05150900042401\n"
"} init/parse-arg (args) #@(name: init/parse-args) #{##(args init/parse-arg init/parse-args init/parse-args/run-repl)\n"
"0E000B00170E010E00110E0004020D0E020E001204010900050E0301\n"
"} init/parse-args (args) #@(name: init/bin) #{##(print/error args init/executable-name init/parse-args module/main :repl)\n"
"0E0019001F0E011105020D0E030E011204010B000D0E041A0524040209000424\n"
"1601\n"
"} init/bin args #@(name: init) #{##(args init/args init/bin)\n"
"0E0005010D0E020E00040101\n"
"} init)\n"
"1A0007010D2407020D0E0324040107040D0E0324040107050D151C07060D1C07\n"
"070D1B07080D1C07090D0E051A0A1A0B1A0C1A0D17370D0E051A0E1A0F1A0C1A\n"
"1017370D0E051A111A121A0C1A1317370D0E051A141A151A0C1A1617370D0E05\n"
"1A171A181A0C1A1917370D0E051A1A1A1B1A0C1A1C17370D0E051A1D1A1E1A0C\n"
"1A1F17370D1A201A211A221707230D1A241A251A261707270D1A281A291A2A17\n"
"072B0D1A2C1A2D1A2E17072F0D1A301A311A321707330D1A341A351A36170737\n"
"16073701\n"
"}#{##((source-path destination-path) #@(documentation: \"Copy a file from SOURCE-PATH to DESTINATION-PATH\" name: file/copy) #{##(spit destination-path slurp source-path)\n"
"0E000E010E020E030401040201\n"
"} file/copy (path) #@(documentation: \"Read in a file using the Nujel reader\" name: file/read) #{##(read file/load path)\n"
"0E000E010E020401040101\n"
"} file/read (path) #@(documentation: \"Read a single value from a file\" name: file/read/single) #{##(file/read path)\n"
"0E000E0104011101\n"
"} file/read/single (path environment) #@(documentation: \"Evaluate a Nujel source file in the current context\" name: file/eval) #{##(eval-in environment root-closure #f do read file/read path)\n"
"0E000E010C0A000D0D0E020C0A00060D1A031A040E050E060E07040104011404\n"
"0201\n"
"} file/eval (path args) #@(documentation: \"Evaluate a Nujel source file in the current context\" name: file/eval-module) #{##(module/resolve-string cat path/without-extension path cwd mod-name module/load mod :environment :type-name resolve exports \"Couldn't load \" \" as a module since it has no exports\" current-lambda :has? :main args)\n"
"0E000E010E020E03040104010E040400040207050D0E060E05040107070D1A08\n"
"1A090E070401200B0007240900060E07010D0E0A1A0B0E070402070B0D0E0B0B\n"
"0007240900160E011A0C0E031A0D040324240E0E04002E042F0D1A0F0E0B1A10\n"
"04020B000F0E0B1A102B0E1104010900042401\n"
"} file/eval-module (path environment) #@(documentation: \"Compile a Nujel source file into optimized object code\" name: file/compile) #{##(do read file/read path source compile* environment #f object-code file/write string/write \"\" cat path/without-extension \".no\")\n"
"1A000E010E020E03040104011407040D0E050E040E060C0A00100D15240D1316\n"
"0C0A00060D1A07040207080D0E090E080B000C0E0A0E0804010900051A0B0E0C\n"
"0E0D0E0304011A0E040204020D0E080101\n"
"} file/compile (path environment base-dir) #@(documentation: \"Compile a Nujel source file into optimized object code\" name: file/compile/module) #{##(:keyword path/without-extension :cut path :length base-dir module-name defmodule/defer def *module* append read file/read source compile* environment #f object-code file/write string/write \"\" cat \".no\")\n"
"1A000E011A020E031A040E05040104020401040107060D1A070E061A081A090E\n"
"06241414140E0A0E0B0E0C0E0304010401240402141414070D0D0E0E0E0D0E0F\n"
"0C0A00100D15240D13160C0A00060D1A10040207110D0E120E110B000C0E130E\n"
"1104010900051A140E150E010E0304011A16040204020D0E110101\n"
"} file/compile/module (#nil) #@(name: file/compile/argv) #{##(last-pair init/args path :index-of \"_modules/\" module file/compile/module :cut file/compile exit)\n"
"0E000E0104011107020D1A030E021A04040207050D0E050200210B001A0E060E\n"
"02241A070E0202000E05020925040304030900090E080E0204010D0E09020004\n"
"0101\n"
"} file/compile/argv (filename) #@(name: file/file?) #{##(file/stat filename :regular-file?)\n"
"0E000E0104011A022B01\n"
"} file/file? (filename) #@(name: file/dir?) #{##(file/stat filename :directory?)\n"
"0E000E0104011A022B01\n"
"} file/dir? (path) #@(name: directory/read-relative) #{##(map ls path (a) #@(name: anonymous) #{##(cat path \"/\" a)\n"
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
"} error (port buf) #@(documentation: \"Reads in a line of user input and returns it\" name: read-line/raw) #{##(i c :u8 buf view :length! 128 :length port char-read :end-of-file)\n"
"020007000D020007010D1A020E03040107040D240900650D240900110D1A051A\n"
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
"} popen/trim cwd +root-working-dir+ *module-path* (name) #@(name: module/loader/filesystem) #{##(:string name name-string System/OS Windows :cut fmt-arg-0 cat \".nuj\" module-path file/read source module def *module* *module-path* path/dirname do read expr compile* current-closure mod)\n"
"1A000E01040107020D0E0202002B022F200B00072409000524010D0E031A0420\n"
"0B00101A050E02020104020502090004240D150E0207060D0E070E061A080402\n"
"1607090D0E0A0E090401070B0D0E0B0B00072409000524010D1A0C1A0D1A0E0E\n"
"01241414141A0D1A0F0E100E090401241414140E110E120E0B0401142E040713\n"
"0D0E140E130E15040004020E1504001D07160D0E160101\n"
"} module/loader/filesystem module/add-loader)\n"
"1A001A011A021707030D1A041A051A061707070D1A081A091A0A17070B0D1A0C\n"
"1A0D1A0E17070F0D1A101A111A121707130D1A141A151A161707170D1A181A19\n"
"1A1A17071B0D1A1C1A1D1A1E17071F0D1A201A211A221707230D1A241A251A26\n"
"1707270D1A281A291A2A17072B0D151A2C1A2D1A2E17072F0D1A301A311A3217\n"
"07331607330D1A341A351A361707370D1A381A391A3A17073B0D0E3B073C0D1A\n"
"3D1A3E1A3F1707400D0E4007070D1A411A421A431707440D1A451A461A471707\n"
"480D0E490E4A0401074B0D1A4C1A4D1A4E17074F0D1A501A511A521707530D0E\n"
"540E55040107560D1A571A581A5917075A0D1A5B1A5C1A5D17075E0D0E540E5F\n"
"040107600D1A611A621A631707640D1A651A661A671707680D1A691A6A1A6B17\n"
"076C0D1A6D1A6E1A6F1707700D1A711A721A731707740D1A751A761A77170778\n"
"0D1A791A7A1A7B17077C0D0E7C077D0D1A7E1A7F1A801707810D1A821A831A84\n"
"1707850D0E86040007870D0E86040007880D1A891A8A1A8B17078C0D0E8D0E8C\n"
"040101\n"
"}#{##((verb host path header) #@(name: http/req*) #{##(header :clone tree/new :Host host :Connection \"close\" :User-Agent \"Nujel/0.1\" join map :keys (k) #@(name: anonymous) #{##(:string k fmt-arg-0 header fmt-arg-1 cat \": \")\n"
"151A000E01040107020D1A000E030E012B040107040D0E050E021A060E040403\n"
"1601\n"
"} \"\\r\\n\" header-lines cat verb \" \" path \" HTTP/1.1\\r\\n\" \"\\r\\n\\r\\n\" req buffer/allocate buf socket/connect fh file/write* :length file/flush* bytes-read bytes-read-now :length! 65536 file/read* file/close* buffer->string raw-res :index-of eosl eoh headers :cut body split status-list read/int status-code ΓεnΣym-1 \":\" eok :keyword key trim v :http-version :status-code :status-message :headers :body)\n"
"0E000B000C1A010E0004010900080E0224040105000D0E001A030E04370D0E00\n"
"1A051A06370D0E001A071A08370D0E090E0A1A0B0E0004011A0C1A0D1A0E1704\n"
"021A0F040207100D0E110E121A130E141A150E101A16040607170D0E18020004\n"
"0107190D0E1A0E0402500402071B0D0E1C0E1B0E171A1D0E17040104030D0E1E\n"
"0E1B04010D0200071F0D020107200D2409002A0D1A210E191A221A1D0E190401\n"
"2504020D0E230E1B0E191A220E1F040405200D0E1F0E2025051F0E202A0B0007\n"
"1C0900041B0AFFCE0D0E240E1B04010D0E250E190E1F040207260D1A270E261A\n"
"0F040207280D0E2802001E0B00082401090004240D1A270E261A16040207290D\n"
"0E2902001E0B00082401090004240D0E02240401072A0D1A2B0E1902040E2925\n"
"0402072C0D0E2D0E250E190E2804021A130402072E0D0E2F0E2E2C040107300D\n"
"150E2D0E250E190E2902020E282504031A0F040207310D240900510D0E311107\n"
"000D1A270E001A32040207330D0E330200210B00301A341A2B0E0002000E3304\n"
"03040107350D0E361A2B0E0002010E33250402040107370D0E2A0E350E373709\n"
"0004240D0E311205310E310AFFB00D24160D0E021A380E2E111A390E301A3A0E\n"
"090E2E12121A1304021A3B0E2A1A3C0E2C040A01\n"
"} http/req* (url) #@(name: http/get) #{##(:index-of url \"https://\" error \"https is unsupported right now\" \"http://\" :cut \"/\" path-start http/req* \"GET\" \"unsupported scheme\")\n"
"1A000E011A0204020200210B000C0E031A040401090004240D1A000E011A0504\n"
"020200210B004A1A060E010207040205010D1A000E011A07040207080D0E0802\n"
"00210B001E0E091A0A1A060E0102000E0804031A060E010E080402040309000D\n"
"0E091A0A0E011A07040301090004240D0E031A0B040101\n"
"} http/get (url filename) #@(name: http/wget) #{##(filename last-pair split url \"/\" http/get res :status-code 299 error \"Couldn't download file\" file/write :body)\n"
"0E000B0007240900120E010E020E031A04040204011105000D0E050E03040107\n"
"060D0E061A072B1A08220B000E0E091A0A0E030402090004240D0E0B0E061A0C\n"
"2B0E00040201\n"
"} http/wget)\n"
"1A001A011A021707030D1A041A051A061707070D1A081A091A0A17070B01\n"
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