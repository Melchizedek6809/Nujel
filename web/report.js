const defaultOptions = {
	log: true,
	metric: "cpu"
};
let options = defaultOptions;

const getLanguageColor = lang => {
	switch(lang){
	case "emacs-lisp": return "#7F5AB6";
	case "newlisp": return "#f7b430";
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

const getRuntimeName = entry => {
	if(entry.language == "nujel"){
		return "Nujel";
	}
	return entry.runtime.split(' ')[0];
};

const getData = (key, filterP, name) => {
	let newestDate = {};
	const data = {};
	for(const run of reportData){
		for(const entry of run){
			if(!entry){continue;}
			if(!filterP(entry)){continue;}
			const runtime = getRuntimeName(entry);
			if(!newestDate[runtime] || (entry.date > newestDate[runtime])){
				newestDate[runtime] = entry.date;
			}
		}
	}
	for(const run of reportData){
		for(const entry of run){
			if(!entry){continue;}
			if(!filterP(entry)){continue;}
			const runtime = getRuntimeName(entry);
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

const goodCatFilter = v => {
	if(!({"scheme": true, "mal": true, "franz-lisp": true}[v.language])){return false;}
	if(v.language == 'scheme'){
	    if(!({"chibi-scheme -q":true, "s9": true, "tinyscheme": true,"mit-scheme-script": true,"scheme48": true}[v.runtime])){return false;}
	}
	return true;
};

const uglyCatFilter = v => {
	if(!({"scheme": true, "lua": true, "c": true, "common-lisp": true, "julia": true, "javascript": true, "python": true, "php": true}[v.language])){return false;}
	if(v.language == 'scheme'){
		if((v.runtime != 'chez --script') && (v.runtime != 'guile') && (v.runtime != 'gosh')){return false;}
	}
	if(v.language == 'common-lisp'){
		if(v.runtime != 'sbcl --script'){return false;}
	}
	if(v.language == 'python'){
		if(v.runtime != 'pypy'){return false;}
	}
	if(v.language == 'javascript'){
		if(v.runtime == 'mujs'){return false;}
	}
	return true;
};

const getHostFilter =     (??, hostname) => v => ??(v) && (v.hostname == hostname);
const getTestcaseFilter = (??, testcase) => v => ??(v) && testcase ? (v.testcase == testcase) : true;
const getCatFilter = (??, cat) => {
	switch(cat){
		case "good": return v => ??(v) && ((v.language == 'nujel') ||    goodCatFilter(v));
		case "bad":  return v => ??(v) && ((v.language == 'nujel') || (!(goodCatFilter(v) || uglyCatFilter(v))));
		case "ugly": return v => ??(v) && ((v.language == 'nujel') ||                        uglyCatFilter(v));
		default:     return ??;
	}
}

const getViews = (key, viewName, cat, hostname, testcase) => {
	const data = [];
	data.push(getData(key, getCatFilter(getHostFilter(getTestcaseFilter(() => true, testcase), hostname), cat), String(viewName)));
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

const yAxisTitle = yAxis => {
	switch(yAxis){
		case "total": return "Total CPU Time";
		case "max-resident": return "Memory Usage (Maximum resident set)";
		default: return "Unknown";
	}
};

const yAxisDescription = yAxis => {
	switch(yAxis){
		case "total": return "CPU Time used to complete benchmark (less is better)";
		case "max-resident": return "Maximum resident set during benchmark run (less is better)";
		default: return "Unknown";
	}
};

const yAxisUnit = yAxis => {
	switch(yAxis){
		case "total": return "milliseconds";
		case "max-resident": return "Bytes";
		default: return "Unknown";
	};
};

const drawSinglePlot = barReport => {
	const log = Boolean(barReport.getAttribute("report-log"));
	const yAxis = String(barReport.getAttribute("report-y"));
	const cat = barReport.getAttribute("report-cat");
	const hostname = barReport.getAttribute("report-hostname") || "yuno";
	const testcase = barReport.getAttribute("report-testcase");
	Plotly.newPlot(barReport, {
		data: getViews(yAxis, yAxisTitle(yAxis), cat, hostname, testcase),
		layout: {
			title: yAxisDescription(yAxis),
			xaxis: {
				title: "Implementation"
			},
			yaxis: {
				title: yAxisUnit(yAxis),
				type: log ? 'log' : null,
				autorange: true
			}
		}
	});
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

	const barReports = document.querySelectorAll(".bar-report");
	let to = 0;
	for(const barReport of barReports){
		to += 10;
		setTimeout(() => drawSinglePlot(barReport), to);
	}
};
setTimeout(analyzeData, 0);
