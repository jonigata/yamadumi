mergeInto(LibraryManager.library, {
    initTextureVault: function(x) {
        document.textureVault = {};
    },
    loadTexture: function(x){
        var filename = Pointer_stringify(x);
        var textureVault = document.textureVault;
        if (!textureVault[x]) {
            console.log("loading " + filename + " start.");
            var image = new Image();
            image.onload = function() {
                console.log("loading " + filename + " done.");
                var gl = document.getElementById('canvas').getContext('webgl');
                var texture = gl.createTexture();
                gl.bindTexture(gl.TEXTURE_2D, texture);
                //gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
                gl.generateMipmap(gl.TEXTURE_2D);
                image.texture = texture;
            };
            image.src = 'data/' + filename;
            textureVault[filename] = image;
        }
    },
    bindTexture: function(x) {
        var filename = Pointer_stringify(x);
        var textureVault = document.textureVault;
        var image = textureVault[filename];
        if (image) {
            var texture = image.texture;
            if (texture) {
                var gl = document.getElementById('canvas').getContext('webgl');
                gl.activeTexture(gl.TEXTURE0);
                gl.bindTexture(gl.TEXTURE_2D, texture);
            }
        }
    },
    initMouse: function() {
        var addMouseEventNative = cwrap(
            'addMouseEvent', 'v', ['number', 'number', 'number', 'number']);
        var canvas = $('#canvas');
        canvas.bind('mousedown', function(e) {
            var offset = $(this).offset(),
                left = e.pageX - offset.left,
                top = e.pageY - offset.top;
            addMouseEventNative(e.which, 0, left, top);
        });
        canvas.bind('mousemove', function(e) {
            var offset = $(this).offset(),
                left = e.pageX - offset.left,
                top = e.pageY - offset.top;
            addMouseEventNative(e.which, 1, left, top);
        });
        canvas.bind('mouseup', function(e) {
            var offset = $(this).offset(),
                left = e.pageX - offset.left,
                top = e.pageY - offset.top;
            addMouseEventNative(e.which, 2, left, top);
        });
        canvas.bind('mouseleave', function(e) {
            var offset = $(this).offset(),
                left = e.pageX - offset.left,
                top = e.pageY - offset.top;
            addMouseEventNative(e.which, 2, left, top);
        });
    },
    initSliders: function() {
        var addSliderEventNative = cwrap(
            'addSliderEvent', 'v', ['number', 'number']);
        $("#stretch-factor").slider({
            min: 0,
            max: 90,
            value: 70,
            change: function(e, ui) {
                console.log(ui.value);
                $('#stretch-factor-value').text(ui.value / 100.0);
                addSliderEventNative(0, ui.value / 100.0);
            }
        });
        $("#restore-factor").slider({
            min: 10,
            max: 100,
            value: 100,
            change: function(e, ui) {
                console.log(ui.value);
                $('#restore-factor-value').text(ui.value / 100.0);
                addSliderEventNative(1, ui.value / 100.0);
            }
        });
        $("#friction").slider({
            min: 0,
            max: 90,
            value: 30,
            change: function(e, ui) {
                console.log(ui.value);
                $('#friction-value').text(ui.value / 100.0);
                addSliderEventNative(2, ui.value / 100.0);
            }
        });
    }
});
