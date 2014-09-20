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
    }
});
