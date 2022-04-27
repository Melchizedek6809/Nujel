let mirror;

const NujelEditor = ($ele, opts) => {
	$ele.classList.add('nujel-editor');
	const {repl, files} = opts;
	let buf = null;

	const $title = $Div("nujel-editor-title", { $parent: $ele });
	const $titleFolderIcon = $Div("nujel-editor-title-folder", {$parent: $title});
	const $titleFolderCrumbs = $Div("nujel-editor-title-crumbs", {$parent: $title});

	const $content = $Div("nujel-editor-content", {
		$parent: $ele
	});
	mirror = new CodeMirror($content, {
		mode:  "nujel",
		inputStyle: "contenteditable",
		lineNumbers: true,
		theme: "ayu-dark"
	});

	const openBuffer = name => {
		buf = files.getBuffer(name);
		mirror.setValue(buf.content);
		$titleFolderCrumbs.innerText = buf.name;
		mirror.focus();
	};

	mirror.on("change", () => {
		buf.content = mirror.getValue();
		files.queueSave(buf.name);
	});

	let evalInFlight = null;
	let evalInFlightCursor = null;
	mirror.on("cursorActivity", () => {
		if(!evalInFlight){return;}
		evalInFlight = null;
		repl.sendForm(mirror.getSelection(), true, true);
		mirror.getDoc().setSelection(evalInFlightCursor);
		evalInFlightCursor = null;
	});

	const evalExpression = () => {
		const source = mirror.getValue();
		const doc = mirror.getDoc();
		let cursor = mirror.getCursor();
		let token = mirror.getTokenAt(cursor);
		if(!evalInFlightCursor){evalInFlightCursor = {line: cursor.line, ch: cursor.ch};}
		if(!token.type){
			while(!token.type){
				if((cursor.line == 0) && (cursor.ch == 0)){break;}
				mirror.execCommand("goCharLeft");
				cursor = mirror.getCursor();
				token = mirror.getTokenAt(cursor);
			}
			setTimeout(evalExpression,0);
			return;
		}
		const splitType = token.type.split(' ');
		if(splitType[0] == 'bracket'){
			const sym = splitType[2];
			const $eles = document.querySelectorAll(`.cm-${sym}`);
			if($eles.length != 2){
				throw new Error("Found more than two braces matching each other, probably not what should happen");
			}
			const sel = window.getSelection();
			evalInFlight = true;
			sel.setBaseAndExtent($eles[0],0,$eles[1],1);
		}else{
			if(splitType[0] == 'string'){
				repl.sendForm('"'+token.string, true, true);
			}else{
				repl.sendForm(token.string, true, true);
			}
		}
	};

	mirror.setOption("extraKeys", {
		"Ctrl-Alt-O":  () => repl.focus(),
	        "Ctrl-Alt-C":  () => evalBuffer(),
	        "Ctrl-Enter": () => evalExpression()
	  });

	const evalBuffer = () => {
		repl.sendForm(mirror.getValue(), false, false);
		visualBell($content);
	};

	const refocus = () => mirror.focus();
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
};
