const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');
let view;
let api;

const rangeX = document.getElementById('range_x');
const rangeY = document.getElementById('range_y');
const rangeA = document.getElementById('range_a');
const rangeL = document.getElementById('range_l');
const rangeLS = document.getElementById('range_ls');
const rangeBF = document.getElementById('range_bf');
const rangeTA = document.getElementById('range_ta');
const rangeD = document.getElementById('range_d');
const valX = document.getElementById('val_x');
const valY = document.getElementById('val_y');
const valA = document.getElementById('val_a');
const valL = document.getElementById('val_l');
const valLS = document.getElementById('val_ls');
const valBF = document.getElementById('val_bf');
const valTA = document.getElementById('val_ta');
const valD = document.getElementById('val_d');
const btnGen = document.getElementById('btn_gen');

Module.onRuntimeInitialized = async _ => {
    api = {
        initFractal: Module.cwrap('initView', 'number', ['number', 'number']),
        drawTree: Module.cwrap('drawTree', 'number', ['number', 'number', 'number', 'number', 'number', 'number', 'number', 'number']),
    };
    
    (rangeX.onchange = _ => { valX.textContent = rangeX.value;  })();
    (rangeY.onchange = _ => { valY.textContent = rangeY.value;  })();
    (rangeA.onchange = _ => { valA.textContent = rangeA.value;  })();
    (rangeL.onchange = _ => { valL.textContent = rangeL.value;  })();
    (rangeLS.onchange = _ => { valLS.textContent = rangeLS.value;  })();
    (rangeBF.onchange = _ => { valBF.textContent = rangeBF.value;  })();
    (rangeTA.onchange = _ => { valTA.textContent = rangeTA.value;  })();
    (rangeD.onchange = _ => { valD.textContent = rangeD.value;  })();

    resizeCanvas();
    drawFractal();
    btnGen.onclick = drawFractal; 
};

function resizeCanvas() {
    const p = api.initFractal(canvas.width, canvas.height);
    const size = canvas.width * canvas.height * 4;
    view = new Uint8ClampedArray(Module.HEAP8.buffer, p, size);
}

function getIntVal(i) {
    return Number.parseInt(i.value);
}

function getFloatVal(i) {
    return Number.parseFloat(i.value);
}

function drawFractal() {
    let x = getFloatVal(rangeX);
    let y = getFloatVal(rangeY);
    let a = getFloatVal(rangeA);
    let l = getFloatVal(rangeL) * Math.sqrt(canvas.width * canvas.width + canvas.height + canvas.height);
    let ls = getFloatVal(rangeLS);
    let bf = getIntVal(rangeBF);
    let ta = getFloatVal(rangeTA);
    let d = getIntVal(rangeD);
    
    console.log(x, y, a, l, ls, bf, ta, d);

    api.drawTree(x, y, a, l, ls, bf, ta, d);
    //api.drawTree(0.5, 1, 270, 200, 0.8, 3, 45.0, 3);
    let img = new ImageData(view, canvas.width, canvas.height);
    ctx.putImageData(img, 0, 0);
}
