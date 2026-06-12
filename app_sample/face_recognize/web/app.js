/*
 * GK7206 Face Detection - Web UI
 * Polls /api/faces and draws bounding boxes + landmarks on canvas overlay
 */

(function () {
    'use strict';

    var mjpegImg = document.getElementById('mjpegStream');
    var canvas = document.getElementById('overlay');
    var ctx = canvas.getContext('2d');
    var container = document.getElementById('videoContainer');

    var npuFpsEl = document.getElementById('npuFps');
    var faceCountEl = document.getElementById('faceCount');
    var statusTextEl = document.getElementById('statusText');

    var POLL_INTERVAL = 100; // ms
    var BOX_COLOR = '#00FF88';
    var BOX_WIDTH = 2;
    var FONT_SIZE = 14;
    var LANDMARK_COLOR = '#00D4FF';
    var LANDMARK_RADIUS = 3;
    /* Landmark connection lines: left-eye, right-eye, nose, left-mouth, right-mouth */
    var LANDMARK_NAMES = ['L-Eye', 'R-Eye', 'Nose', 'L-Mouth', 'R-Mouth'];

    function resizeCanvas() {
        // Match canvas to displayed image size
        var rect = mjpegImg.getBoundingClientRect();
        canvas.width = rect.width;
        canvas.height = rect.height;
        canvas.style.width = rect.width + 'px';
        canvas.style.height = rect.height + 'px';
    }

    function drawLandmarks(face, cw, ch) {
        if (!face.landmarks || face.landmarks.length < 5) return;

        // Draw landmark points
        for (var k = 0; k < 5; k++) {
            var lx = face.landmarks[k].x * cw;
            var ly = face.landmarks[k].y * ch;

            ctx.beginPath();
            ctx.arc(lx, ly, LANDMARK_RADIUS, 0, Math.PI * 2);
            ctx.fillStyle = LANDMARK_COLOR;
            ctx.fill();

            // White border for visibility
            ctx.strokeStyle = '#FFFFFF';
            ctx.lineWidth = 1;
            ctx.stroke();
        }

        // Draw connecting lines between landmarks (eye-line, nose-center)
        ctx.strokeStyle = 'rgba(0, 212, 255, 0.5)';
        ctx.lineWidth = 1;
        ctx.setLineDash([3, 3]);

        // Left eye → Right eye
        ctx.beginPath();
        ctx.moveTo(face.landmarks[0].x * cw, face.landmarks[0].y * ch);
        ctx.lineTo(face.landmarks[1].x * cw, face.landmarks[1].y * ch);
        ctx.stroke();

        // Nose point
        ctx.beginPath();
        ctx.moveTo(face.landmarks[2].x * cw, face.landmarks[2].y * ch);
        ctx.stroke();

        // Left mouth → Right mouth
        ctx.beginPath();
        ctx.moveTo(face.landmarks[3].x * cw, face.landmarks[3].y * ch);
        ctx.lineTo(face.landmarks[4].x * cw, face.landmarks[4].y * ch);
        ctx.stroke();

        ctx.setLineDash([]);
    }

    function drawFaces(data) {
        resizeCanvas();
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        if (!data || !data.faces || data.faces.length === 0) {
            faceCountEl.textContent = '0';
            return;
        }

        faceCountEl.textContent = data.faces.length;
        npuFpsEl.textContent = data.fps || '--';

        var cw = canvas.width;
        var ch = canvas.height;

        for (var i = 0; i < data.faces.length; i++) {
            var face = data.faces[i];
            // Coordinates are normalized (0.0 ~ 1.0)
            var x = face.x * cw;
            var y = face.y * ch;
            var w = face.w * cw;
            var h = face.h * ch;

            // Draw bounding box
            ctx.strokeStyle = BOX_COLOR;
            ctx.lineWidth = BOX_WIDTH;
            ctx.strokeRect(x, y, w, h);

            // Draw facial landmarks (from O-Net)
            drawLandmarks(face, cw, ch);

            // Draw label
            var label = (face.score * 100).toFixed(0) + '%';
            ctx.font = FONT_SIZE + 'px monospace';
            var textWidth = ctx.measureText(label).width;
            ctx.fillStyle = BOX_COLOR;
            ctx.fillRect(x, y - FONT_SIZE - 4, textWidth + 8, FONT_SIZE + 4);
            ctx.fillStyle = '#000000';
            ctx.fillText(label, x + 4, y - 4);
        }
    }

    function pollFaces() {
        fetch('/api/faces')
            .then(function (response) { return response.json(); })
            .then(function (data) {
                drawFaces(data);
                statusTextEl.textContent = '运行中 (3-stage MTCNN)';
                statusTextEl.className = 'value status-running';
            })
            .catch(function () {
                statusTextEl.textContent = '连接中...';
                statusTextEl.className = 'value status-connecting';
            });
    }

    // Start polling
    mjpegImg.onload = function () {
        resizeCanvas();
    };

    window.addEventListener('resize', resizeCanvas);

    // Initial resize
    setTimeout(resizeCanvas, 500);
    setInterval(pollFaces, POLL_INTERVAL);
    pollFaces();
})();
