function create_shader(gl, id) {
    var shader;
    var ele = document.getElementById(id);
    
    var typeMap = {
        'x-shader/x-vertex': gl.VERTEX_SHADER,
        'x-shader/x-fragment': gl.FRAGMENT_SHADER
    };
    
    shader = gl.createShader(typeMap[ele.type]);
    gl.shaderSource(shader, ele.innerHTML);
    gl.compileShader(shader);
    
    debugger;
    if (gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        return shader;
    }
    else {
        alert(gl.getShaderInfoLog(shader));
    }
}

function create_program(gl, vs, fs) {
    var prg = gl.createProgram();

    gl.attachShader(prg, vs);
    gl.attachShader(prg, fs);

    gl.linkProgram(prg);

    if (gl.getProgramParameter(prg, gl.LINK_STATUS)) {
        gl.useProgram(prg);
        return prg;
    }
    else {
        alert(gl.getProgramInfoLog(prg));
    }
}

function create_vbo (gl, data) {
    var vbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(data), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    return vbo;
}

function create_ibo (gl, data) {
    var ibo = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Int16Array(data), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
    return ibo;
}

//------------------------------------------------------------------------------

function doit() {

    var cv = document.getElementById('canvas');
    cv.width = window.innerWidth;
    cv.height = window.innerHeight;
    var gl = cv.getContext('webgl') || cv.getContext('experimental-webgl');

    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clearDepth(1.0);

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    var vs = create_shader(gl, 'vs');
    var fs = create_shader(gl, 'fs');

    var prg = create_program(gl, vs, fs);

    var position = [
        0.0, 1.0, 0.0,
            -1.0, 0.0, 0.0, 
        1.0, 0.0, 0.0
    ];
    for (var i = 0 ; i < position.length ; i++) {
        position[i] = position[i] * 160 + 320;
    }


    var color = [
        1.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 1.0
    ];

    var index = [
        0, 1, 2
    ];

    var vPosition = create_vbo(gl, position);
    var vColor = create_vbo(gl, color);
    var ibo = create_ibo(gl, index);

    //生成したprg（プログラムオブジェクト）から、各種attribute変数のインデックス番号を取得
    var attLocation = [];
    attLocation[0] = gl.getAttribLocation(prg, 'position');
    attLocation[1] = gl.getAttribLocation(prg, 'color');

    var attDiv = [3, 4];

    gl.bindBuffer(gl.ARRAY_BUFFER, vPosition);
    gl.enableVertexAttribArray(attLocation[0]);
    //第一引数は頂点属性の番号（index）、第二引数はひとつの頂点の要素数、第三引数は要素のデータ型
    gl.vertexAttribPointer(attLocation[0], attDiv[0], gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, vColor);
    gl.enableVertexAttribArray(attLocation[1]);
    gl.vertexAttribPointer(attLocation[1], attDiv[1], gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);


    //生成したprg（プログラムオブジェクト）から、各種uniform変数のインデックス番号を取得
    var uniLocation = gl.getUniformLocation(prg, 'mvpMatrix');

    (function loop() {
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
        
        gl.drawElements(gl.TRIANGLES, index.length, gl.UNSIGNED_SHORT, 0);
        gl.flush();
        
        setTimeout(loop, 32);
    }());
}
