const NujelREPL = (ele, nuj) => {
	const output = document.createElement("DIV");
	output.classList.add("nujel-repl-buffer");
	const input = document.createElement("INPUT");
	input.classList.add("nujel-repl");
	output.readOnly = true;
	ele.appendChild(output);
	ele.appendChild(input);

	let editorFocus = null;
	let evalBuffer = null;
	let history = [];
	let historyCurrent = null;
	let historySelection = -1;

	const historyLoad = () => {
		const raw = localStorage.getItem("nujel-repl-history");
		if(raw){history = JSON.parse(raw);}
	};
	historyLoad();

	const historySave = () => {
		localStorage.setItem("nujel-repl-history",JSON.stringify(history))
	}

	const historyAdd = (line) => {
		history.push(line);
		historySave();
	}

	const historyPrev = () => {
		if(historyCurrent === null){historyCurrent = input.value;}
		if(historySelection++ >= history.length-1){historySelection = history.length-1;};

		input.value = history[history.length - historySelection - 1];
	}

	const historyNext = () => {
		if(--historySelection < 0){
			historySelection = -1;
			if(historyCurrent !== null){
				input.value = historyCurrent;
			}
		}else{
			input.value = history[history.length - historySelection - 1];
		}
	}

	const outputWriteInput = (data) => {
		data = data.replace("\r\n","\n");
		for(const line of data.split("\n")){
			if(line === ""){continue;}
			const lineElement = document.createElement("DIV");
			lineElement.innerText = line;
			lineElement.classList.add("nujel-repl-input");
			output.appendChild(lineElement);
		}
		output.scrollTop = output.scrollHeight;
		historyCurrent = null;
		historySelection = -1;
	};

	const createAnsiTag = status => {
		const arr = ["ansi-seq"];
		if(status?.ansiFG){arr.push(`ansi-fg="${status.ansiFG}"`);}
		if(status?.ansiBG){arr.push(`ansi-bg="${status.ansiBG}"`);}
		if(status?.ansiBold){arr.push(`ansi-bold`);}
		return `</ansi-seq><${arr.join(" ")}>`;
	};

	const parseSingleAnsiSequence = (ansiStatus,codePoint) => {
		if(codePoint === "0"){
			return {};
		}else if(codePoint === "1"){
			ansiStatus.ansiBold = true;
			return ansiStatus;
		}
		const codeVal = codePoint | 0;
		if((codeVal >= 30) && (codeVal <= 37)){
			ansiStatus.ansiFG = codeVal - 30;
			return ansiStatus;
		}
		if((codeVal >= 40) && (codeVal <= 47)){
			ansiStatus.ansiFG = codeVal - 40;
			return ansiStatus;
		}
		return ansiStatus;
	}

	const parseAnsiSequences = strIn => {
		let out = "";
		let code = "";
		let inSequence = false;

		let ansiStatus = {};

		const chars = strIn.split("");
		for(const char of chars){
			switch(char){
				default:
					if(!inSequence){
						out += char;
					}else if(char == "m"){
						inSequence = false;
						if(code.substr(0,1) !== "["){continue;}
						for(const codePoint of code.substr(1).split(";")){
							ansiStatus = parseSingleAnsiSequence(ansiStatus,codePoint);
						}
						code = "";
						out += createAnsiTag(ansiStatus);
					}else{
						code += char;
					}
					break;
				case "":
					inSequence = true;
					break;
			}
		}
		return `<ansi-seq>${out}</ansi-seq>`;
	};

	const outputWrite = (data) => {
		for(const line of data.split("\r")){
			if(line === ""){continue;}
			const lineElement = document.createElement("DIV");
			lineElement.innerHTML = parseAnsiSequences(line);
			lineElement.classList.add("nujel-repl-line");
			output.appendChild(lineElement);
		}
		output.scrollTop = output.scrollHeight;
	};

	const runForm = (form, echo=true, echoResult=true) => {
		const result = nuj.run(form);
		if(echo){outputWriteInput(form);}
		outputWrite(nuj.read());
		if(echoResult){outputWrite(result);}
		if(echo){historyAdd(form);}
		return result;
	}

	input.addEventListener("keypress",(ev) => {
		if(ev.code === "Enter"){
			runForm(input.value);
			input.value = "";
		}
	});

	input.addEventListener("keydown",(ev) => {
		switch(ev.code){
			case "ArrowUp":
				historyPrev();
				break;
			case "ArrowDown":
				historyNext();
				break;
		}
		if((ev.keyCode == 90) && ev.ctrlKey){
			ev.preventDefault();
			console.log("OI!");
			editorFocus && editorFocus();
		}
		if((ev.keyCode == 67) && ev.ctrlKey && ev.altKey){
			ev.preventDefault();
			evalBuffer();
		}
	});
	ele.addEventListener("click", () => input.focus());
	output.addEventListener("click", e => e.stopPropagation());

	input.focus();
	outputWrite(nuj.read());

	return {
		focus: () => input.focus(),
		eval: form => nuj.run(form),
		sendForm: runForm,
		setEditorFocus: λ => editorFocus = λ,
		setEvalBuffer: λ => evalBuffer = λ
	}
};
