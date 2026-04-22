dom_output.readOnly = true;
let g_prefix = "";
let g_input_start = 0;

dom_code.addEventListener('keydown', (e) => {
    if (e.key === "Tab") {
        e.preventDefault();
        //console.log("Tabbed")
        let cursor = dom_code.selectionStart;
        dom_code.value = dom_code.value.slice(0, cursor) + "    " + dom_code.value.slice(cursor);
        dom_code.setSelectionRange(cursor + 4, cursor + 4);
    }
});

dom_output.addEventListener('keydown', (e) => {
// Block arrow keys, Home, End, etc.
    const blockedKeys = ['ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown', 'Home', 'End'];
    if (blockedKeys.includes(e.key)) {
        e.preventDefault();
    }

    if (e.key === "Enter" && !dom_output.readOnly) {
        e.preventDefault();
        //console.log("INPUT:", dom_output.value.slice(g_input_start));
        if (inputCallback) {
            inputCallback(dom_output.value.slice(g_input_start));
            outf("\n");
            inputCallback = null; // reset after use
            dom_output.readOnly = true;
        }
        //dom_output.value += "\n";
    }
});

dom_output.addEventListener("mousedown", (e) => {
    e.preventDefault();
    dom_output.focus();
    dom_output.setSelectionRange(dom_output.value.length, dom_output.value.length);
});


function outf(text) {
    dom_output.value += text;
    g_prefix = dom_output.value;
}

function builtinRead(x) {
    if (Sk.builtinFiles === undefined || Sk.builtinFiles["files"][x] === undefined)
        throw "File not found: '" + x + "'";
    return Sk.builtinFiles["files"][x];
}

let inputCallback = null;

//TODO: this function should be called when user presses enter when textarea is highlighted
/*
function submitInput() {
    const userText = document.getElementById("user-input").value;
    if (inputCallback) {
        inputCallback(userText);
        inputCallback = null; // reset after use
    }
    document.getElementById("user-input").value = ""; // clear input box
                                                      //TODO: disable typing in dom_output
}
*/

function inputFunction(promptText) {
    dom_output.readOnly = false;
    dom_output.focus();
    dom_output.setSelectionRange(dom_output.value.length, dom_output.value.length);
    g_input_start = dom_output.value.length;
    return new Promise((resolve) => {
            inputCallback = resolve;
            dom_output.value += promptText;
            g_prefix = dom_output.value;
            });
}

function runit() {
    dom_output.value = '';
    g_prefix = '';
    Sk.configure({ output: outf, read: builtinRead, inputfun: inputFunction, inputfunTakesPrompt: false });
    Sk.misceval.asyncToPromise(() => {
            return Sk.importMainWithBody("<stdin>", false, dom_code.value, true);
            }).then(
                () => console.log("Success"),
                (err) => { 
                    console.error(err.toString()); 
                    dom_output.value += err.toString();
                }
                );
}


// output functions are configurable.  This one just appends some text
// to a pre element.
function outf(text) { 
    //var mypre = document.getElementById("output"); 
    //mypre.innerHTML = mypre.innerHTML + text; 

    dom_output.value += text;
    g_prefix = dom_output.value;
} 
/*
function builtinRead(x) {
    if (Sk.builtinFiles === undefined || Sk.builtinFiles["files"][x] === undefined)
            throw "File not found: '" + x + "'";
    return Sk.builtinFiles["files"][x];
}

// Here's everything you need to run a python program in skulpt
// grab the code from your textarea
// get a reference to your pre element for output
// configure the output function
// call Sk.importMainWithBody()
function runit() { 
   var prog = document.getElementById("dom_code").value; 
   var mypre = document.getElementById("output"); 
   mypre.innerHTML = ''; 
   Sk.pre = "output";
   Sk.configure({output:outf, read:builtinRead}); 
   (Sk.TurtleGraphics || (Sk.TurtleGraphics = {})).target = 'mycanvas';
   var myPromise = Sk.misceval.asyncToPromise(function() {
       return Sk.importMainWithBody("<stdin>", false, prog, true);
   });
   myPromise.then(function(mod) {
       console.log('success');
   },
       function(err) {
       console.log(err.toString());
   });
} 
*/
