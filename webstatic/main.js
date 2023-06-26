


// global function to set global options. see below.
var JSCodeHighlight;

(function(){
// Options
var gopts = {
	rootClass: 'js-code-highlight-root',
	inlineRootClass: 'inline',
	multilineRootClass: 'multiline',
	titleClass: 'title',
	lineClass: 'line',
	gutterClass: 'line-nums',
	codeBlockClass: 'code-block',
	font: "Courier New",
	multilineSelector: "foobar",
	inlineSelector: "ipre, c, m",
	bareSelector: "c",
	failsafe: 10000,
	defaultLanguage: 'c',
	defaultTagLanguage: { m: 'asm', c: 'c' },
	languageClassPrefix: 'lang-',
};

JSCodeHighlight = function(o) {
	for(var k in o) {
		gopts[k] = o[k];
	}
};


// Highlighting Rules
// The only non-anonymous group is the one captured
// name: "foo" will be added as a css class to the element.
var rule_list = {
	none: [],
	c: [
		{name: 'whitespace', re: /^(\s+)/ },
		{name: 'preproc', re:  /^(#[a-zA-Z]+.*)(?:\n|$)/ },
		{name: 'comment', re:  /^(\/\/.*)(\n|$)/ },
		{name: 'comment', re:  /^(\/\*(?:.|\n)*\*\/)/m },
		{name: 'type', re:    /^((?:const|extern|int|inline|restrict|void|volatile|float|char|double|unsigned|signed|short|long|static|struct|union|enum|auto|register|[a-z_0-9]+_t)\**)(?:\W|$)/ },
		{name: 'keyword', re: /^(if|for|else|while|do|switch|return|break|continue|default|case|goto|typedef|sizeof|offsetof)(?:\W|$)/ },
		{name: 'string', re:   /^("(?:[^"]*|\\.)*")/ },
		{name: 'charlit', re:   /^('(?:[^']*|\\.)*')/ },
		{name: 'number', re:   /^([-+]?[0-9]*\.?[0-9]+e?[0-9]*[ulfb]*)/i },
		{name: 'number', re:   /^([-+]?0x[0-9a-fA-F]*\.?[0-9a-fA-F]+e?[0-9a-fA-F]*[ulfb]*)/i },
		{name: 'punct', re:    /^((?:&gt;|&lt;|&amp;)+|[\(\)\[\]{}\|\.,\+=\-?\/\\\*^%:;!~]+)/ },
		{name: 'ident', re:    /^(?:([a-zA-Z_][a-zA-Z_0-9]*)(?:\W|$))/ },
	],
	asm: [ // intel style
		{name: 'whitespace', re: /^(\s+)/ },
		{name: 'comment', re:  /^(;.*)(\n|$)/ },
		{name: 'keyword', re: /^((?:(?:[qd]?word|byte)\s+(?:ptr))|(offset (?:flat)?))(?:\W|$)/i },
		{name: 'directive', re:  /^(\.(?:align|ascii|bcd|bss|[248]?byte|comm|data|double|even|ext|file|float|glabl|group|hidden|ident|lcomm|local|long|popsection|previous|pushsection|quad|rel|section|set|size|skip|sleb128|string|symbolic|tbss|tcomm|tdata|text|type|uleb128|value|weak|zero))/i },
		{name: 'label', re:  /^(\.?[a-z_][a-z0-9_]*:)/i },
		{name: 'label', re:  /^(\.[a-z_][a-z0-9_]*:?)/i },
		{name: 'register', re: /^([RE]?[ABCD]X|[RE]?[SBI]P|[RE]?[SD]I|[ABCD][HL]|[SB]PL|[SD]IL|R[89][BWD]?|R1[0-5][BWD]?|EFLAGS|FLAGS)(?:\W|$)/i },
		{name: 'simd-register', re: /^([xyz]?mm\d\d?)(?:\W|$)/i },
		{name: 'string', re:   /^("(?:[^"]*|\\.)*")/ },
		{name: 'number', re:   /^([-+]?0[xb][0-9a-fA-F]*\.?[0-9a-fA-F]+e?[0-9a-fA-F]*[ulfb]*)/i },
		{name: 'number', re:   /^([-+]?[0-9]*\.?[0-9]+e?[0-9]*[ulfb]*)/i },
		{name: 'punct', re:    /^([\[\],\+\-:]+)/ },
		{name: 'ident', re:    /^([a-zA-Z_][a-zA-Z_0-9]*)(?:\W|$)/ },
	]
};



window.addEventListener('load', function() {
	
	// utility functions
	
	function min(a,b) {return a > b ? b : a }
	
	function leadingTabs(s) {
		for(var i = 0; i < s.length && s[i] == "\t"; i++);
		
		return i;
	}
	
	function eat(s, re, cl, out_lines) {
		var m = s.l.match(re);
		if(m) {
//  				console.log(cl, m);
			var t = m[1];
			out_lines.push(
				t.split('\n')
					.map(function(x) { return '<span class="'+cl+'">'+x+'</span>'})
					.join('\n')
			);
			s.l = s.l.slice(t.length);
			return true;
		}
		return false;
	}
	
	function processLine(s, rules) {
		var k = {l: s};
		var j = 0;
		
		var ol = []
		while(k.l.length > 0) {
			if(j++ > gopts.failsafe) return;
			
			// preserve tags
			var m = k.l.match(/^<\/?[^>]*>/);
			if(m) {
				ol.push(m[0]);
				console.log(m[0]);
				k.l = k.l.slice(m[0].length);
				continue;
			}
			
			/*
			// preserve &..;
			var m = k.l.match(/^&[a-z]{1,4};/);
			if(m) {
				ol.push(m[0]);
				console.log(m[0]);
				k.l = k.l.slice(m[0].length);
				continue;
			}
			*/
			
			// process the rules
			var br = false;
			for(var r of rules) {
				if(j++ > gopts.failsafe) return;
				if(eat(k, r.re, r.name, ol)) {
					br = true;
					break;
				}
			}
			
			if(br) continue;
			
			ol.push(k.l[0]);
			k.l = k.l.slice(1);
		}
		
		return ol;
	}
	
	
	
	// multiline pre's
	/*
	var pres = document.querySelectorAll(gopts.multilineSelector);
	
	for(var pre of pres) {
		var text = pre.innerHTML;
		var lines = text.split("\n");
		
		var lang = pre.getAttribute('lang') || gopts.defaultLanguage;
		var rules = rule_list[lang];
		if(!rules) rules = rule_list['none'];
		
		var start_line = parseFloat(pre.getAttribute('ln') || 1);
		
		// find out how many tabs to trim on the left
		var least_tabs = 999999999;
		var trailing_empty = 0;
		for(var l of lines) {
			var n = leadingTabs(l);
			if(l[n] !== undefined) {
				least_tabs = min(least_tabs, n);
				if(l[n] != ' ') {
					trailing_empty = 0;
				}
			}
			
			trailing_empty++;
		}
		
		lines = lines.slice(0, -trailing_empty+1);
		
		var out_lines = [];
		for(var l of lines) { 
			l = l.slice(least_tabs);
			out_lines.push(l);
		}
		// all this messy joining and splitting is to handle multi-line matches like comments
		//   but also put each code line inside its own span
		var txt = out_lines.join("\n");
		var hl = processLine(txt, rules);
		
		out_lines = hl.join("").split("\n");
		
		
		var parent = document.createElement('div');
		parent.classList.add(gopts.rootClass);
		parent.classList.add(gopts.multilineRootClass);
		parent.classList.add(gopts.languageClassPrefix + lang);
		parent.style.display = "block";
		parent.style.position = "relative";
		parent.style.overflowX = "auto";
		parent.style.overflowY = "auto";
		
		pre.replaceWith(parent);
		
		var title = pre.getAttribute('title')
		if(title) {
			var tn = document.createElement('div');
			tn.classList.add(gopts.titleClass);
			tn.innerHTML = title;
			parent.append(tn);
		}
		
		var scrollp = document.createElement('div');
		scrollp.style.clear = "both";
		parent.appendChild(scrollp);
		
		var line_nums = document.createElement('span');
		line_nums.classList.add(gopts.gutterClass);
		line_nums.style.display = "block";
		line_nums.style.float = "left";
		line_nums.style.fontFamily = gopts.font;
		line_nums.style.whiteSpace = "pre";
		line_nums.style.userSelect = "none";
		for(var i = 0; i < lines.length; i++) {
			var nl = document.createElement('span');
			nl.classList.add(gopts.lineClass);
			nl.innerHTML = (i + start_line) + "\n";
			line_nums.append(nl);
		}
		scrollp.append(line_nums);
		
		var code_block = document.createElement('div');
		code_block.classList.add(gopts.codeBlockClass);
		code_block.style.position = "absolute";
		code_block.style.fontFamily = gopts.font;
		code_block.style.whiteSpace = "pre";
		code_block.style.left = line_nums.offsetWidth;
		
//		getComputedStyle
		for(var l of out_lines) {
			var nl = document.createElement('span');
			nl.classList.add(gopts.lineClass);
			nl.innerHTML = l + "\n";
			code_block.append(nl);
		}
		
		scrollp.append(code_block);
	} 
	
	*/
	
	// inline pre's
	/*
	var pres = document.querySelectorAll(gopts.inlineSelector);
	for(var pre of pres) {
		var extra = "";
		var src = pre.innerHTML.replace(/^\s+/, '').replace(/\s+$/, '')
		
		var lang = pre.getAttribute('lang') || gopts.defaultLanguage;
		var rules = rule_list[lang];
		if(!rules) rules = rule_list['none'];
		
		var line_num = pre.getAttribute('ln');
		if(line_num) {
			extra = '<span class="line-nums">'+line_num+'</span>';
		}
		
		var parent = document.createElement('span');
		parent.style.display = "inline-block";
		parent.style.whiteSpace = "pre";
		parent.style.fontFamily = gopts.font;
		parent.classList.add(gopts.rootClass);
		parent.classList.add(gopts.inlineRootClass);
		parent.classList.add(gopts.languageClassPrefix + lang);
		parent.innerHTML = extra + processLine(src, rules).join('');
		
		pre.replaceWith(parent);
	}
	
	*/
	// bare lines
	var bare = document.querySelectorAll(gopts.bareSelector);
	for(var pre of bare) {
		var extra = "";
		var src = pre.innerHTML;//.replace(/^\s+/, '').replace(/\s+$/, '')
		
		var lang = pre.getAttribute('lang') || gopts.defaultLanguage;
		var rules = rule_list[lang];
		if(!rules) rules = rule_list['c'];
		/*
		var parent = document.createElement('span');
		parent.style.display = "inline-block";
		parent.style.whiteSpace = "pre";
		parent.style.fontFamily = gopts.font;
		parent.classList.add(gopts.rootClass);
		parent.classList.add(gopts.inlineRootClass);
		parent.classList.add(gopts.languageClassPrefix + lang);
		parent.innerHTML = extra + processLine(src, rules).join('');
		*/
		pre.innerHTML = extra + processLine(src, rules).join('');
		
//		pre.replaceWith(parent);
	}

});
})();
