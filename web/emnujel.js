let args = window.location.search+'';
if(args.charAt(0) == '?'){
	args = args.substr(1);
}
args = args.split('&');
for(var i=0;i<args.length;i++){
	args[i] = '-'+args[i];
}

let nujel;
let stdoutBuffer = "";

var Module = {
	arguments: args,
	preRun: [],
	postRun: [function(){
		const nujelRun = Module.cwrap('run','string',['string']);
		const nujelRead = () => {
			const ret = stdoutBuffer;
			stdoutBuffer = "";
			return ret;
		};
		nujel = (line) => {
			return nujelRun(line);
		};
		const repl = NujelREPL(document.getElementById("nujel-repl-wrapper"),{run: nujelRun, read: nujelRead});
		const files = NujelFilebrowser(document.getElementById("nujel-filebrowser-wrapper"),{repl});
		NujelEditor(document.getElementById("nujel-editor-wrapper"),{file: "*scratch-buffer*", files, repl});
	}],
	print: function(text) {
		stdoutBuffer += text;
	},
	printErr: function(text) {
		stdoutBuffer += text;
	},
	setStatus: function(text) {},
	totalDependencies: 0,
	monitorRunDependencies: function(left) {
		this.totalDependencies = Math.max(this.totalDependencies, left);
	}
};
