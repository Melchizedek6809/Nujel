const NujelEditor = (ele, opts) => {
	ele.classList.add('nujel-editor');
	let activeFile = null;
	const buffers  = {};
	const {repl, files} = opts;
	let buf        = null;

	const $title   = document.createElement("DIV");
	$title.classList.add("nujel-editor-title");
	const $titleFolderIcon = document.createElement("DIV");
	$titleFolderIcon.classList.add("nujel-editor-title-folder");
	const $titleFolderCrumbs = document.createElement("DIV");
	$titleFolderCrumbs.classList.add("nujel-editor-title-crumbs");
	$title.append($titleFolderIcon);
	$title.append($titleFolderCrumbs);
	const $content = document.createElement("DIV");
	$content.classList.add("nujel-editor-content");
	$content.setAttribute("contenteditable",true);
	$content.setAttribute("spellcheck","false");
	ele.append($title);
	ele.append($content);


	const openBuffer = name => {
		buf = files.getBuffer(name);
		$content.innerText = buf.content;
		$titleFolderCrumbs.innerText = buf.name;
	};

	const evalBuffer = () => {
		repl.sendForm($content.innerText, false);
		$content.classList.add("visual-bell");
		$content.offsetHeight;
		$content.classList.remove("visual-bell");
	};

	$content.addEventListener("input", e => {
		buf.content = $content.innerText;
		files.queueSave(buf.name);
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
			evalBuffer();
		}
	});

	$content.addEventListener("blur", () => {
		buf.range = window.getSelection().getRangeAt(0);
	});

	const refocus = () => {
		$content.focus();
		const sel = window.getSelection();
		sel.removeAllRanges();
		if(!buf.range){
			buf.range = new Range();
			const node = $content.childNodes[$content.childNodes.length-1];
			buf.range.setStartAfter(node);
			buf.range.setEndAfter(node);
			buf.range.collapse();
		}
		sel.addRange(buf.range);
	};

	const getCurrentBuffer = () => buf;

	openBuffer(opts.file);
	repl.setEditorFocus(refocus);
	repl.setEvalBuffer(evalBuffer);
	files.setSwitchBuffer(openBuffer);

	return {
		evalBuffer,
		openBuffer,
		getCurrentBuffer
	};
}
