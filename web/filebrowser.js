const NujelFilebrowser = (ele, opts) => {
	let buf = null;
	let buffers = {};
	let $dirs = {};
	let switchBuffer = null;
	const {repl} = opts;
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
	};

	const $getActions = (curBuf,$node) => {
		const $ret = $Div("filebrowser-actions");
		const $eval = $Div("filebrowser-eval", {
			$parent: $ret,
			text: "Evaluate Buffer",
			onClick: e => {
				e.stopPropagation();
				evalBuffer(curBuf);
				visualBell($node);
			}
		});
		const $delete = $Div("filebrowser-delete", {
			$parent: $ret,
			text: "Delete Buffer",
			onClick: e => {
				e.stopPropagation();
				if(!confirm(`Are you sure you want to delete ${curBuf.name}?`)){return;}
				unlink(curBuf.name);
				if(buf === curBuf){switchBuffer("*scratch-buffer*");}
				refreshDOM();
			}
		});
		return $ret;
	};

	const $getDirActions = (name, $node) => {
		const $ret = $Div("filebrowser-actions");
		const $eval = $Div("filebrowser-eval", {
			$parent: $ret,
			text: "Evaluate Directory",
			onClick: e => {
				e.stopPropagation();
				visualBell($node);
				evalDir(name);
			}
		});
		const $delete = $Div("filebrowser-delete", {
			$parent: $ret,
			text: "Delete Buffer",
			onClick: e => {
				e.stopPropagation();
				if(!confirm(`Are you sure you want to delete ${name} and all its buffers?`)){return;}
				const pred = c => c.name.startsWith(name);
				Object.values(buffers).filter(pred).forEach(unlink);
				refreshDOM();
			}
		});
		return $ret;
	};

	const getDirNode = name => {
		if($dirs[name]){return $dirs[name];}
		if(name == ""){return $files;}
		const segments = name.split("/");
		const dirPath = segments.slice(0,segments.length - 1).join('/');
		const $dir = getDirNode(dirPath);
		const $node = $Div("nujel-filebrowser-directory", {
			$parent: $dir
		});
		$node.append($getDirActions(name,$node));
		const $title = $Div("nujel-filebrowser-directory-title", {
			text: segments[segments.length-1],
			$parent: $node,
			onClick: () => {
				$title.classList.toggle("active");
				$newDir.classList.toggle("active");
			}
		});
		const $newDir = $Div("nujel-filebrowser-directory-children", {$parent: $node});
		$dirs[name] = $newDir;
		return $newDir;
	};

	const refreshDOM = n => {
		$files.innerHTML = "";
		$dirs = {};
		const bufferList = Object.values(buffers).sort((a,b) => a.name < b.name ? -1 : 1);
		bufferList.forEach(curBuf => {
			const segments = curBuf.name.split("/");
			const dirName  = getDirname(curBuf.name);
			const $dir     = getDirNode(dirName);
			const $node    = $Div("nujel-filebrowser-buffer", {
				classList: (buf === curBuf ? ["active"] : null),
				text: segments[segments.length-1],
				onClick: () => {
					switchBuffer(curBuf.name);
					buf = curBuf;
					for(const child of $files.querySelectorAll('.nujel-filebrowser-buffer.active')){
						child.classList.remove('active');
					}
					$node.classList.add("active");
				}
			});
			curBuf.$dom = $node;
			$dir.append($node);
			$node.append($getActions(curBuf,$node));
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
	const unlink = name => {
		if(typeof name === 'string'){
			delete buffers[name];
			localStorage.removeItem(`${LSPrefix}${name}`);
		}else if(name.name){
			unlink(name.name);
		}else{
			throw new Error("Can't unlink that");
		}
	};
	const evalBuffer = curBuf => repl.sendForm(curBuf.content ,false , false);
	const evalBufferQueue = q => {
		if(!q.length){return;}
		evalBuffer(q.pop());
		setTimeout(() => evalBufferQueue(q), 0);
	};
	const evalDir = name => {
		const pred = (c) => c.name.startsWith(name);
		evalBufferQueue(Object.values(buffers).filter(pred).reverse());

	};
	const save = name => {
		if(buffers[name].remote){return;}
		const data = JSON.stringify(buffers[name]);
		localStorage.setItem(`${LSPrefix}${name}`, data);
		saveTimeoutHandle = null;
	};
	const queueSave = name => {
		if(buffers[name].remote){return;}
		saveTimeoutHandle && clearTimeout(saveTimeoutHandle);
		saveTimeoutHandle = setTimeout(() => save(name), 100);
	};
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
};
