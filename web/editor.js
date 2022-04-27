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
	const mirror = new CodeMirror($content, {
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

	mirror.setOption("extraKeys", {
		"Ctrl-Alt-O": () => repl.focus(),
		"Ctrl-Alt-C": () => evalBuffer()
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
