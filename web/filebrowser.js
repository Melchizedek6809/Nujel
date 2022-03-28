const NujelFilebrowser = (ele, opts) => {
	let buffers = {};
	let switchBuffer = null;
	const LSPrefix = "nujel-buffer-";
	const $title = document.createElement("DIV");
	$title.classList.add("nujel-filebrowser-titlebar");
	const $files = document.createElement("DIV");
	$files.classList.add("nujel-filebrowser-files");
	ele.append($title);
	$title.innerText = "Buffers";
	ele.append($files);

	const refreshDOM = () => {
		Object.values(buffers).filter(buf => !buf.$dom).forEach(buf => {
			const $node = document.createElement("DIV");
			$node.classList.add("nujel-filebrowser-buffer");
			$files.append($node);
			$node.innerText = buf.name;
			$node.addEventListener("click",() => {
				switchBuffer(buf.name);
			});
			buf.$dom = $node;
		});
	};

	const newBuffer = (name, content) => ({
		name,
		$node: null,
		range: null,
		content: content || `; This is a scratch buffer, nothing here will be saved
; But you can use it to experiment with Nujel!
;
; Ctrl-Alt-c Evaluates the entire buffer
; Ctrl-Z switches between the Editor and REPL.`});
/*
; Alt-Return sends the current top-level Form to the repl
; Ctrl-Return for the current form
*/
	const loadBuffer = name => {
		try {
			const entry = JSON.parse(localStorage.getItem(`${LSPrefix}${name}`));
			buffers[name] = newBuffer(name, entry.content);
		} catch(e) { /* Could have invalid JSON, if that is the case we just continue */}
	};
	const loadBuffers = () => Object.keys(localStorage).filter(k => k.startsWith(LSPrefix)).map(k => k.substring(LSPrefix.length)).forEach(loadBuffer);

	const getBuffer = name => buffers[name] || (buffers[name] = newBuffer(name));
	let saveTimeoutHandle = null;
	const save = name => {
		const data = JSON.stringify(buffers[name])
		localStorage.setItem(`${LSPrefix}${name}`, data);
		saveTimeoutHandle = null;
		console.log(data);
	}
	const queueSave = name => {
		saveTimeoutHandle && clearTimeout(saveTimeoutHandle);
		saveTimeoutHandle = setTimeout(() => save(name), 100);
		console.log(name);
	}
	const setSwitchBuffer = fun => switchBuffer = fun;

	loadBuffers();
	refreshDOM();

	return {
		newBuffer,
		getBuffer,
		queueSave,
		setSwitchBuffer,
		save
	};
}