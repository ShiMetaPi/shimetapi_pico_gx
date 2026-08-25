// Rock Paper Scissors YOLOv8 Web UI
const API_GESTURE = '/api/gesture/current';
const API_DETECTIONS = '/api/detections';
const API_GAME_START = '/api/game/start';
const API_GAME_SCORE = '/api/game/score';
const API_STATUS = '/api/status';

let currentDetections = [];
let videoWidth = 640;
let videoHeight = 480;

async function fetchJson(url) {
    try {
        const r = await fetch(url);
        return await r.json();
    } catch(e) {
        console.error('Fetch error:', e);
        return null;
    }
}

async function updateDetections() {
    const det = await fetchJson(API_DETECTIONS);
    if (det) {
        currentDetections = det.detections || [];
        updateDetectionList(currentDetections);
        drawBoundingBoxes(currentDetections);
    }
}

async function updateGesture() {
    const g = await fetchJson(API_GESTURE);
    if (g) {
        document.getElementById('gesture-info').innerHTML =
            `<p class="gesture-big">${g.gesture}</p>
             <p>置信度: ${(g.confidence * 100).toFixed(1)}%</p>`;
    } else {
        document.getElementById('gesture-info').innerHTML =
            `<p class="gesture-big">未检测</p><p>等待手势输入...</p>`;
    }
}

async function updateStatus() {
    const s = await fetchJson(API_STATUS);
    if (s) {
        document.getElementById('fps-info').textContent =
            `FPS: ${s.fps} | 检测数: ${s.detections || 0}`;
    }
}

async function startGame() {
    const r = await fetchJson(API_GAME_START);
    if (r) {
        let resultText = '';
        let resultColor = '#4ecca3';
        if (r.result === 'win') {
            resultText = '🎉 你赢了!';
            resultColor = '#4ecca3';
        } else if (r.result === 'lose') {
            resultText = '😔 你输了!';
            resultColor = '#e94560';
        } else if (r.result === 'tie') {
            resultText = '🤝 平局!';
            resultColor = '#f9ca24';
        } else {
            resultText = '游戏进行中...';
        }

        document.getElementById('game-result').innerHTML =
            `<p>你: ${r.player} vs 电脑: ${r.computer}</p>
             <p class="result-big" style="color:${resultColor}">${resultText}</p>`;
        updateScore();
    }
}

async function updateScore() {
    const s = await fetchJson(API_GAME_SCORE);
    if (s) {
        document.getElementById('score-info').innerHTML =
            `<p>玩家: ${s.player} | 电脑: ${s.computer} | 局数: ${s.rounds}</p>`;
    }
}

function updateDetectionList(detections) {
    const listDiv = document.getElementById('det-list');
    if (!detections || detections.length === 0) {
        listDiv.innerHTML = '<p style="color:#888">暂无检测目标</p>';
        return;
    }

    listDiv.innerHTML = detections.map((det, idx) => {
        const badgeClass = `badge-${det.class.toLowerCase()}`;
        return `<div class="det-item ${badgeClass}">
            <span class="det-class">${det.class}</span>
            <span class="det-score">${(det.score * 100).toFixed(1)}%</span>
        </div>`;
    }).join('');
}

function drawBoundingBoxes(detections) {
    const canvas = document.getElementById('bbox-overlay');
    const videoImg = document.getElementById('mjpeg-stream');

    if (!canvas || !videoImg || !detections || detections.length === 0) {
        if (canvas) canvas.style.display = 'none';
        return;
    }

    // Set canvas dimensions to match video
    canvas.style.display = 'block';
    canvas.style.position = 'absolute';
    canvas.style.top = '0';
    canvas.style.left = '0';
    canvas.style.width = videoImg.style.width || '100%';
    canvas.style.height = videoImg.style.height || 'auto';

    // Scale factor from model input (640x640) to display size
    const displayWidth = videoImg.clientWidth || videoWidth;
    const displayHeight = videoImg.clientHeight || videoHeight;
    const scaleX = displayWidth / 640;
    const scaleY = displayHeight / 640;

    canvas.width = displayWidth;
    canvas.height = displayHeight;

    const ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Draw each detection box
    detections.forEach((det, idx) => {
        const x = det.bbox[0] * scaleX;
        const y = det.bbox[1] * scaleY;
        const w = (det.bbox[2] - det.bbox[0]) * scaleX;
        const h = (det.bbox[3] - det.bbox[1]) * scaleY;

        // Choose color based on class
        const colors = {
            'Rock': '#e94560',
            'Paper': '#4ecca3',
            'Scissors': '#f9ca24'
        };
        const color = colors[det.class] || '#00ff00';

        // Draw box
        ctx.strokeStyle = color;
        ctx.lineWidth = 3;
        ctx.strokeRect(x, y, w, h);

        // Draw label background
        const label = `${det.class} ${(det.score * 100).toFixed(0)}%`;
        ctx.fillStyle = color;
        const textWidth = ctx.measureText(label).width + 10;
        ctx.fillRect(x, y - 25, textWidth, 25);

        // Draw label text
        ctx.fillStyle = '#fff';
        ctx.font = 'bold 14px Arial';
        ctx.fillText(label, x + 5, y - 8);
    });
}

function init() {
    // Periodic updates
    setInterval(updateDetections, 500);
    setInterval(updateStatus, 1000);

    // Initial updates
    updateDetections();
    updateGesture();
    updateStatus();
    updateScore();

    // Handle video stream errors
    const videoStream = document.getElementById('mjpeg-stream');
    videoStream.onerror = function() {
        this.src = '/mjpeg?' + new Date().getTime();
    };
}

document.addEventListener('DOMContentLoaded', init);