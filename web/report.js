const defaultOptions = {
	log: true,
	metric: "cpu"
};
let options = defaultOptions;

const getLanguageColor = lang => {
	switch(lang){
	case "scheme": return "#cc2244";
	case "julia": return "#9358a4";
	case "common-lisp": return "#369844";
	case "javascript": return "#ff8d11";
	case "erlang": return "#aa0130";
	case "kuroko": return "#dc3545";
	case "lua": return "#030380";
	case "ruby": return "#cc342d";
	case "php": return "#787cb4";
	case "perl": return "#0073a1";
	case "dart": return "#04599c";
	case "janet": return "#aa87de";
	case "python": return "#ffda4c";
	case "racket": return "#9f1d20";
	case "nujel": return "#000";
	default: return "#88AAee";
	}
}

const getDistinct = key => {
	const ret = {};
	for(const run of reportData){
		for(const entry of run){
			if(!entry){continue;}
			if(!entry[key]){continue;}
			ret[entry[key]] = true;
		}
	}
	return Object.keys(ret);
}

const getArchitectures = () => getDistinct("architecture");
const getCommits = () => getDistinct("git-head");
const getLanguages = () => getDistinct("language");
const getTestcases = () => getDistinct("testcase");
const getOS = () => getDistinct("os");
const getDates = () => getDistinct("date");

const initConfig = () => {
	const $logScale = document.querySelector("#log-scale");
	const $addView = document.querySelector("#add-view");
	const $views = document.querySelector("#report-views");

	const addView = () => {
		const $new = document.createElement("DIV");
		$new.classList.add("report-view");
		$new.innerHTML = `
		<label>μArch: <select name="architecture"><option value="">ALL</option>${ getArchitectures().map(a => `<option value="${a}">${a}</option>`).join("") }</select></label>
		<label>OS: <select name="os"><option value="">ALL</option>${ getOS().map(a => `<option value="${a}">${a}</option>`).join("") }</select></label>
		<label>Date: <select name="date"><option value="">ALL</option>${ getDates().map(a => `<option value="${a}">${a}</option>`).join("") }</select></label>
		<label>Commit: <select name="git-head"><option value="">ALL</option>${ getCommits().map(a => `<option value="${a}">${a}</option>`).join("") }</select></label>
		<label>Language: <select name="language"><option value="">ALL</option>${ getLanguages().map(a => `<option value="${a}">${a}</option>`).join("") }</select></label>
		<label>Test: <select name="testcase"><option value="">ALL</option>${ getTestcases().map(a => `<option value="${a}">${a}</option>`).join("") }</select></label>
		<button class="remove-button">Remove view</button>
		`;
		for(const $select of $new.querySelectorAll("select")){
			$select.addEventListener("change", () => { setTimeout(analyzeData, 0); });
		}
		for(const $but of $new.querySelectorAll("button.remove-button")){
			$but.addEventListener("click", () => { $new.remove(); setTimeout(analyzeData, 0); });
		}
		$views.append($new);

		setTimeout(analyzeData, 0);
	};

	$addView.addEventListener("click", e => { addView(); });
	$logScale.addEventListener("change", e => {
		options.log = $logScale.checked;
		setTimeout(analyzeData, 0);
	});
	addView();
};

const getData = (key, filterP, name) => {
	let newestDate = {};
	const data = {};
	for(const run of reportData){
		for(const entry of run){
			if(!entry){continue;}
			if(!filterP(entry)){continue;}
			const runtime = entry.runtime == "./nujel" ? "nujel" : entry.runtime;
			if(!newestDate[runtime] || (entry.date > newestDate[runtime])){
				newestDate[runtime] = entry.date;
			}
		}
	}
	for(const run of reportData){
		for(const entry of run){
			if(!entry){continue;}
			if(!filterP(entry)){continue;}
			const runtime = entry.runtime == "./nujel" ? "nujel" : entry.runtime;
			if(entry.date != newestDate[runtime]){continue;}
			if(!data[runtime]){
				data[runtime] = {
					"language": entry.language,
					"value": 0,
					"count": 0
				};
			}
			data[runtime].value += entry[key] * 1000.0;
			data[runtime].count += 1;
		}
	}
	const ret = {x:[],y:[],type:'bar',name ,marker:{color:[]}};
	for(const runtime in data){
		ret.x.push(runtime);
		ret.y.push(data[runtime].value / data[runtime].count);
		ret.marker.color.push(getLanguageColor(data[runtime].language));
	}
	return ret;
};

const getViews = (key, name) => {
	const data = [];
	for(const $view of document.querySelectorAll(".report-view")){
		let filterP = v => true;
		let filterName = [];
		for(const $select of $view.querySelectorAll("select")){
			if(!$select.value){continue;}
			const key = $select.getAttribute("name");
			const value = $select.value;
			filterName.push(value);
			const lastP = filterP;
			filterP = entry => lastP(entry) && entry[key] == value;
		}
		const n = filterName.length ? filterName.join("-") : "ALL";
		data.push(getData(key, filterP, name+' '+n));
	}
	return data;
}

const getSingleViews = key => {
	const data = {};
	for(const run of reportData){
		for(const entry of run){
			if(!entry){continue;}
			if(entry.language != "nujel"){continue;}
			const k = `${entry.testcase}-${entry.hostname}-${entry.architecture}`;
			if(!data[k]){data[k]={};}
			const d = entry.date;
			if(!data[k][d]){
				data[k][d] = {
					"value": 0,
					"count": 0
				};
			}
			data[k][d].value += entry[key] * 1000.0;
			data[k][d].count += 1;
		}
	}
	const retArr = [];
	for(const name in data){
		const ret = {x:[],y:[],type: 'line', name};
		const dates = Object.keys(data[name]).sort();
		for(const d of dates){
			ret.x.push(d);
			const v = data[name][d].value / data[name][d].count;
			ret.y.push(v ? v : 1.0);
		}
		retArr.push(ret);
	}
	return retArr;
};

const analyzeData = () => {
	Plotly.newPlot("report-cpu-time", {
		data: getSingleViews("total"),
		layout: {
			title: "Nujel CPU Time used to complete benchmark (less is better)",
			xaxis: {
				title: "Date"
			},
			yaxis: {
				title: "milliseconds",
				type: options.log ? 'log' : null,
				autorange: true
			}
		}
	});

	Plotly.newPlot("report-memory-time", {
		data: getSingleViews("max-resident"),
		layout: {
			title: "Nujel Maximum resident set during benchmark run (less is better)",
			xaxis: {
				title: "Date"
			},
			yaxis: {
				title: "Bytes",
				type: options.log ? 'log' : null,
				autorange: true
			}
		}
	});

	Plotly.newPlot("report-cpu", {
		data: getViews("total", "CPU Time"),
		layout: {
			title: "CPU Time used to complete benchmark (less is better)",
			xaxis: {
				title: "Implementation"
			},
			yaxis: {
				title: "milliseconds",
				type: options.log ? 'log' : null,
				autorange: true
			}
		}
	});

	Plotly.newPlot("report-memory", {
		data: getViews("max-resident", "Memory Max"),
		layout: {
			title: "Maximum resident set during benchmark run (less is better)",
			xaxis: {
				title: "Implementation"
			},
			yaxis: {
				title: "Bytes",
				type: options.log ? 'log' : null,
				autorange: true
			}
		}
	});
};

setTimeout(initConfig, 0);
