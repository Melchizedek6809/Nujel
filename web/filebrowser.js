const NujelFilebrowser = (ele, opts) => {
	let buf = null;
	let buffers = {};
	let $dirs = {};
	let switchBuffer = null;
	const LSPrefix = "nujel-buffer-";
	const $title = document.createElement("DIV");
	$title.classList.add("nujel-filebrowser-titlebar");
	const $files = document.createElement("DIV");
	$files.classList.add("nujel-filebrowser-files");
	ele.append($title);
	$title.innerText = "Buffers";
	ele.append($files);

	const getDirname = name => {
		const segments = name.split("/");
		return segments.slice(0,segments.length - 1).join('/');
	}

	const getDirNode = name => {
		if($dirs[name]){return $dirs[name];}
		if(name == ""){return $files;}
		const segments = name.split("/");
		const dirPath = segments.slice(0,segments.length - 1).join('/');
		const $dir = getDirNode(dirPath);
		const $node = document.createElement("DIV");
		const $title = document.createElement("DIV");
		$title.innerText = segments[segments.length-1];
		$title.classList.add("nujel-filebrowser-directory-title")
		const $newDir = document.createElement("DIV");
		$newDir.classList.add("nujel-filebrowser-directory-children");
		$node.append($title);
		$node.append($newDir);
		$title.addEventListener("click", () => {
			$title.classList.toggle("active");
			$newDir.classList.toggle("active");
		});
		$node.classList.add("nujel-filebrowser-directory");
		$dirs[name] = $newDir;
		$dir.append($node);
		return $newDir;
	};

	const refreshDOM = n => {
		$files.innerHTML = "";
		$dirs = {};
		Object.values(buffers).sort((a,b) => a.name - b.name).forEach(curBuf => {
			const $node = document.createElement("DIV");
			$node.classList.add("nujel-filebrowser-buffer");
			if(buf === curBuf){
				$node.classList.add("active");
			}
			getDirNode(getDirname(curBuf.name)).append($node);
			const segments = curBuf.name.split("/");
			$node.innerText = segments[segments.length-1];
			$node.addEventListener("click",() => {
				switchBuffer(curBuf.name);
				buf = curBuf;
				for(const child of $files.querySelectorAll('.nujel-filebrowser-buffer.active')){
					child.classList.remove('active');
				}
				$node.classList.add("active");
			});
			curBuf.$dom = $node;
		});
	};

	const newBuffer = (name, content, remote=false) => ({
		name,
		remote,
		$node: null,
		range: null,
		content: content || `; This is a scratch buffer,
; you can use it to experiment with Nujel!
;
; Ctrl-Alt-c Evaluates the entire buffer
; Ctrl-Z switches between the Editor and REPL.`});
/*
; Alt-Return sends the current top-level Form to the repl
; Ctrl-Return for the current form
*/

	const loadFilesystem = async () => {
		try {
			const resp = await fetch("./filesystem.json");
			const data = await resp.json();
			for(const k in data){
				const v = data[k];
				buffers[v.name] = newBuffer(v.name, v.content, true);
			}
			refreshDOM();
		} catch(e) {}
	};
	loadFilesystem();

	const loadBuffer = name => {
		try {
			const entry = JSON.parse(localStorage.getItem(`${LSPrefix}${name}`));
			buffers[name] = newBuffer(name, entry.content);
		} catch(e) { /* Could have invalid JSON, if that is the case we just continue */}
	};
	const loadBuffers = () => Object.keys(localStorage).filter(k => k.startsWith(LSPrefix)).map(k => k.substring(LSPrefix.length)).forEach(loadBuffer);

	const getBuffer = name => buf = (buffers[name] || (buffers[name] = newBuffer(name)));
	let saveTimeoutHandle = null;
	const save = name => {
		if(buffers[name].remote){return;}
		const data = JSON.stringify(buffers[name])
		localStorage.setItem(`${LSPrefix}${name}`, data);
		saveTimeoutHandle = null;
	}
	const queueSave = name => {
		if(buffers[name].remote){return;}
		saveTimeoutHandle && clearTimeout(saveTimeoutHandle);
		saveTimeoutHandle = setTimeout(() => save(name), 100);
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