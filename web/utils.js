const $New = (opts = {}) => {
	const {$parent, className, classList, text, html, tagName, onClick} = opts;
	if(!tagName){
		throw new Error('You need to provide a valid tagName!!!');
	}
	const $ret = document.createElement(tagName);
	if(className){$ret.classList.add(className);}
	if(classList){
		const adder = c => $ret.classList.add(c);
		classList.forEach(adder);
	}
	if($parent){$parent.append($ret);}
	if(text){$ret.innerText = text;}
	if(html){$ret.innerHTML = html;}
	if(onClick){$ret.addEventListener("click", onClick);}
	return $ret;
};
const $Div = (className, opts = {}) => $New({tagName: "DIV", className:className, ...opts});

const visualBell = $ele => {
	$ele.classList.add("visual-bell");
	$ele.offsetHeight;
	$ele.classList.remove("visual-bell");
};
