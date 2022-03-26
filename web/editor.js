const NujelEditor = (ele, opts) => {
	let activeFile = null;
	const buffers = {};
	const {repl} = opts;
	let buf = null;
	const $content = ele.querySelector(".nujel-editor-content");
	const $title = ele.querySelector(".nujel-editor-title");

	const newBuffer = name => ({
		title: name,
		range: null,
		content: `; This is a scratch buffer, nothing here will be saved
; But you can use it to experiment with Nujel!
;
; Ctrl-Alt-c Evaluates the entire buffer
; Ctrl-Z switches between the Editor and REPL.`});
/*
; Alt-Return sends the current top-level Form to the repl
; Ctrl-Return for the current form
*/

	const openBuffer = name => {
		if(!buffers[name]){buffers[name] = newBuffer(name);}
		buf = buffers[name];
		$content.innerText = buf.content;
		$title.innerText = buf.title;
	};

	$content.addEventListener("input", e => {

	});
	$content.addEventListener("keydown", e => {
		if ((e.keyCode == 10 || e.keyCode == 13)){
			if(e.ctrlKey){
				console.log("Ctrl!!");
			}
			if(e.altKey){
				console.log("Meta!!");
			}
		}
		if((e.keyCode == 90) && e.ctrlKey){
			e.preventDefault();
			repl.focus();
		}
		if((e.keyCode == 67) && e.ctrlKey && e.altKey){
			e.preventDefault();
			repl.sendForm($content.innerText,false);
		}
	});

	$content.addEventListener("blur", () => {
		buf.range = window.getSelection().getRangeAt(0);
	});

	const refocus = () => {
		$content.focus();
		const sel = window.getSelection();
		sel.removeAllRanges();
		buf.range && sel.addRange(buf.range);
	};

	openBuffer(opts.file);
	repl.setEditorFocus(refocus);
}